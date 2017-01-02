//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1997
// 
// uBaseCoroutine.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sat Sep 27 16:46:37 1997
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Dec 29 12:05:56 2016
// Update Count     : 553
//
// This  library is free  software; you  can redistribute  it and/or  modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software  Foundation; either  version 2.1 of  the License, or  (at your
// option) any later version.
// 
// This library is distributed in the  hope that it will be useful, but WITHOUT
// ANY  WARRANTY;  without even  the  implied  warranty  of MERCHANTABILITY  or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
// 
// You should  have received a  copy of the  GNU Lesser General  Public License
// along  with this library.
// 

#define __U_KERNEL__
#define __U_PROFILE__
#define __U_PROFILEABLE_ONLY__


#include <uC++.h>
#ifdef __U_PROFILER__
#include <uProfiler.h>
#endif // __U_PROFILER__
//#include <uDebug.h>


//######################### uBaseCoroutine #########################


namespace __cxxabiv1 {
    // This routine must be called manually since our exception throwing interferes with normal deallocation
    extern "C" void __cxa_free_exception(void *vptr) throw();

    // These two routines allow proper per-coroutine exception handling
    extern "C" __cxa_eh_globals *__cxa_get_globals_fast() throw() {
	return &uThisCoroutine().ehGlobals;
    }

    extern "C" __cxa_eh_globals *__cxa_get_globals() throw() {
	return &uThisCoroutine().ehGlobals;
    }
} // namespace


uBaseCoroutine::UnwindStack::UnwindStack( bool e ) : exec_dtor( e ) {
} // uBaseCoroutine::UnwindStack::UnwindStack

uBaseCoroutine::UnwindStack::~UnwindStack() {
    if ( exec_dtor && ! std::uncaught_exception() ) { 	// if executed as part of an exceptional clean-up do nothing, otherwise terminate is called
	__cxxabiv1::__cxa_free_exception( this );	// if handler terminates and 'safety' is off, clean up the memory for old exception
	_Throw UnwindStack( true );			// and throw a new exception to continue unwinding
    } // if
} // uBaseCoroutine::UnwindStack::~UnwindStack


void uBaseCoroutine::unwindStack() {
    if ( ! cancelInProgress_ ) {			// do not cancel if we're already cancelling
	cancelInProgress_ = true;
	// NOTE: This throw fails and terminates the application if it occurs in the middle of a destructor triggered by
	// an exception.  While not a serious restriction now, it could be one when time-slice polling is introduced
	_Throw UnwindStack( true );			// start the cancellation unwinding
    } // if
} // uBaseCoroutine::unwindStack


void uBaseCoroutine::createCoroutine() {
    errno_ = 0;
    state = Start;
    notHalted = true;					// must be a non-zero value so detectable after memory is scrubbed

    last = nullptr;					// see ~uCoroutineDestructor
#ifdef __U_DEBUG__
    currSerialOwner = nullptr;				// for error checking
    currSerialCount = 0;
#endif // __U_DEBUG__

    // exception handling / cancellation

    handlerStackTop = handlerStackVisualTop = nullptr;
    resumedObj = nullptr;
    topResumedType = nullptr;
    DEStack = nullptr;
    unexpectedRtn = uEHM::unexpected;			// initialize default unexpected routine
    unexpected = false;

    memset( &ehGlobals, 0, sizeof( ehGlobals ) );

    cancelled_ = false;
    cancelInProgress_ = false;
    cancelState = CancelEnabled;
    cancelType = CancelPoll;				// not used yet, but makes Pthread cancellation easier

#ifdef __U_PROFILER__
    // profiling

    profileTaskSamplerInstance = nullptr;
#endif // __U_PROFILER__
} // uBaseCoroutine::createCoroutine


// SKULLDUGGERY: __errno_location may be defined with attribute "const" so its result can be stored and reused. So in
// taskCxtSw, the thread-local address may be stored in a register on the front side of the context switch, but on the
// back side of the context switch the processor may have changed so the stored value is wrong. This routine forces
// another call to __errno_location on the back side of the context switch.

#if ! defined( __U_ERRNO_FUNC__ )
static int *my_errno_location() __attribute__(( noinline )); // prevent unwrapping
static int *my_errno_location() {
    return &errno;
} // my_errno_location
#endif // ! __U_ERRNO_FUNC__


void uBaseCoroutine::taskCxtSw() {			// switch between a task and the kernel
    uBaseCoroutine &coroutine = uThisCoroutine();	// optimization
    uBaseTask &currTask = uThisTask();

#ifdef __U_PROFILER__
    if ( currTask.profileActive && uProfiler::uProfiler_builtinRegisterTaskBlock ) { // uninterruptable hooks
	(*uProfiler::uProfiler_builtinRegisterTaskBlock)( uProfiler::profilerInstance, currTask );
    } // if
#endif // __U_PROFILER__

    coroutine.setState( Inactive );			// set state of current coroutine to inactive

#ifdef __U_DEBUG_H__
    uDebugPrt( "(uBaseCoroutine &)%p.taskCxtSw, coroutine:%p, coroutine.SP:%p, coroutine.storage:%p, storage:%p\n",
	       this, &coroutine, coroutine.stackPointer(), coroutine.storage, storage );
#endif // __U_DEBUG_H__

#if ! defined( __U_ERRNO_FUNC__ )
    errno_ = errno;					// save
#endif // ! __U_ERRNO_FUNC__
    coroutine.save();					// save user specified contexts

    uSwitch( coroutine.context, context );		// context switch to kernel

    coroutine.restore();				// restore user specified contexts

#if ! defined( __U_ERRNO_FUNC__ )
    *my_errno_location() = errno_;			// restore
#endif // ! __U_ERRNO_FUNC__

    coroutine.setState( Active );			// set state of new coroutine to active
    currTask.setState( uBaseTask::Running );

#ifdef __U_PROFILER__
    if ( currTask.profileActive && uProfiler::uProfiler_builtinRegisterTaskUnblock ) { // uninterruptable hooks
	(*uProfiler::uProfiler_builtinRegisterTaskUnblock)( uProfiler::profilerInstance, currTask );
    } // if
#endif // __U_PROFILER__
} // uBaseCoroutine::taskCxtSw


void uBaseCoroutine::corCxtSw() {			// switch between two coroutine contexts
    uBaseCoroutine &coroutine = uThisCoroutine();	// optimization
    uBaseTask &currTask = uThisTask();

#ifdef __U_DEBUG__
    // reset task in current coroutine?
    if ( coroutine.currSerialCount == currTask.currSerialLevel ) {
	coroutine.currSerialOwner = nullptr;
    } // if
    
    // check and set for new owner
    if ( currSerialOwner != &currTask ) {
	if ( currSerialOwner != nullptr  ) {
	    if ( &currSerialOwner->getCoroutine() != this ) {
		uAbort( "Attempt by task %.256s (%p) to activate coroutine %.256s (%p) currently executing in a mutex object owned by task %.256s (%p).\n"
			"Possible cause is task attempting to logically change ownership of a mutex object via a coroutine.",
			currTask.getName(), &currTask, this->getName(), this, currSerialOwner->getName(), currSerialOwner );
	    } else {
		uAbort( "Attempt by task %.256s (%p) to resume coroutine %.256s (%p) currently being executed by task %.256s (%p).\n"
			"Possible cause is two tasks attempting simultaneous execution of the same coroutine.",
			currTask.getName(), &currTask, this->getName(), this, currSerialOwner->getName(), currSerialOwner );
	    } // if
	}  else {
	    currSerialOwner = &currTask;
	    currSerialCount = currTask.currSerialLevel;
	} // if
    } // if
#endif // __U_DEBUG__

#ifdef __U_PROFILER__
    if ( currTask.profileActive && uProfiler::uProfiler_registerCoroutineBlock ) {
	(*uProfiler::uProfiler_registerCoroutineBlock)( uProfiler::profilerInstance, currTask, *this );
    } // if
#endif // __U_PROFILER__

    THREAD_GETMEM( This )->disableInterrupts();

    coroutine.setState( Inactive );			// set state of current coroutine to inactive

#ifdef __U_DEBUG_H__
    uDebugPrt( "(uBaseCoroutine &)%p.corCxtSw, coroutine:%p, coroutine.SP:%p\n",
	       this, &coroutine, coroutine.stackPointer() );
#endif // __U_DEBUG_H__
    coroutine.save();					// save user specified contexts
    currTask.currCoroutine = this;			// set new coroutine that task is executing

#if defined( __U_MULTI__ ) && defined( __U_SWAPCONTEXT__ )
#   if defined( __linux__ ) && defined( __ia64__ )
	((ucontext_t *)(context))->uc_mcontext.sc_gr[13] = THREAD_GETMEM( threadPointer );
#   else
	#error uC++ : internal error, unsupported architecture
#   endif
#endif // __U_MULTI__ && __U_SWAPCONTEXT__

    uSwitch( coroutine.context, context );		// context switch to specified coroutine

    coroutine.restore();				// restore user specified contexts
    coroutine.setState( Active );			// set state of new coroutine to active

    THREAD_GETMEM( This )->enableInterrupts();

#ifdef __U_PROFILER__
    if ( uThisTask().profileActive && uProfiler::uProfiler_registerCoroutineUnblock ) {
	(*uProfiler::uProfiler_registerCoroutineUnblock)( uProfiler::profilerInstance, uThisTask() );
    } // if
#endif // __U_PROFILER__
} // uBaseCoroutine::corCxtSw


void uBaseCoroutine::corFinish() {			// resumes the coroutine that first resumed this coroutine
    notHalted = false;
#ifdef __U_DEBUG__
    if ( ! starter_->notHalted ) {			// check if terminated
	    uAbort( "Attempt by coroutine %.256s (%p) to resume back to terminated starter coroutine %.256s (%p).\n"
		    "Possible cause is terminated coroutine's main routine has already returned.",
		    uThisCoroutine().getName(), &uThisCoroutine(), starter_->getName(), starter_ );
    } // if
#endif // __U_DEBUG__
    starter_->corCxtSw();
    // CONTROL NEVER REACHES HERE!
    uAbort( "(uBaseCoroutine &)%p.corFinish() : internal error, attempt to return.", this );
} // uBaseCoroutine::corFinish


const char *uBaseCoroutine::setName( const char *name ) {
    const char *prev = name;
    uBaseCoroutine::name = name;

#ifdef __U_PROFILER__
    if ( uThisTask().profileActive && uProfiler::uProfiler_registerSetName ) { 
	(*uProfiler::uProfiler_registerSetName)( uProfiler::profilerInstance, *this, name ); 
    } // if
#endif // __U_PROFILER__
    return prev;
} // uBaseCoroutine::setName

const char *uBaseCoroutine::getName() const {
    // storage might be uninitialized or scrubbed
    return name == nullptr
#ifdef __U_DEBUG__
	     || name == (const char *)-1		// only scrub in debug
#endif // __U_DEBUG__
	? "*unknown*" : name;
} // uBaseCoroutine::getName


void uBaseCoroutine::setCancelState( CancellationState state ) {
#ifdef __U_DEBUG__
    if ( this != &uThisCoroutine() && uBaseCoroutine::state != Start ) {
	uAbort( "Attempt to set the cancellation state of coroutine %.256s (%p) by coroutine %.256s (%p).\n"
		"A coroutine/task may only change its own cancellation state.",
		getName(), this, uThisCoroutine().getName(), &uThisCoroutine() );
    } // if
#endif // __U_DEBUG__
    cancelState = state;
} // uBaseCoroutine::setCancelState

void uBaseCoroutine::setCancelType( CancellationType type ) {
#ifdef __U_DEBUG__
    if ( this != &uThisCoroutine() && uBaseCoroutine::state != Start ) {
	uAbort( "Attempt to set the cancellation state of coroutine %.256s (%p) by coroutine %.256s (%p).\n"
		"A coroutine/task may only change its own cancellation state.",
		getName(), this, uThisCoroutine().getName(), &uThisCoroutine() );
    } // if
#endif // __U_DEBUG__
    cancelType = type;
} // uBaseCoroutine::setCancelType


uBaseCoroutine::Failure::Failure( const char *const msg ) : uKernelFailure( msg ) {
} // uBaseCoroutine::Failure::Failure


uBaseCoroutine::UnhandledException::UnhandledException( uBaseEvent *cause, const char *const msg ) :
	Failure( msg ), cause( cause ), origFailedCor( uThisCoroutine() ), multiple( false ) {
    cleanup = true;
    uEHM::strncpy( origFailedCorName, origFailedCor.getName(), uEHMMaxName );
} // uBaseCoroutine::UnhandledException::UnhandledException

uBaseCoroutine::UnhandledException::UnhandledException( UnhandledException *cause ) :
	Failure( cause->message() ), cause( cause ), origFailedCor( cause->origFailedCor ), multiple( true ) {
    cleanup = true;
    uEHM::strncpy( origFailedCorName, cause->origFailedCorName, uEHMMaxName );
} // uBaseCoroutine::UnhandledException::UnhandledException


uBaseCoroutine::UnhandledException::UnhandledException( const uBaseCoroutine::UnhandledException &ex ) : origFailedCor( ex.origFailedCor ) { 
    memcpy( this, &ex, sizeof(*this) );			// relies on all fields having trivial copy constructors
    ex.cleanup = false;					// then prevent the original from deleting the cause
} // uBaseCoroutine::UnhandledException::UnhandledException

uBaseCoroutine::UnhandledException::~UnhandledException() {
    if ( cleanup ) {
	delete cause;					// clean up the stored exception object
    } // if
} // uBaseCoroutine::UnhandledException::~UnhandledException

const uBaseCoroutine &uBaseCoroutine::UnhandledException::origSource() const {
    return origFailedCor;
} // uBaseCoroutine::origSource

const char *uBaseCoroutine::UnhandledException::origName() const {
    return origFailedCorName;
} // uBaseCoroutine::origName

void uBaseCoroutine::UnhandledException::triggerCause() {
    if ( cause != nullptr ) cause->reraise();
} // uBaseCoroutine::UnhandledException::triggerCause

void uBaseCoroutine::UnhandledException::defaultTerminate() const {
    if ( ! multiple ) {
	uAbort( "(uBaseCoroutine &)%p : Unhandled exception in coroutine %.256s raised non-locally from resumed coroutine %.256s (%p), which was terminated due to %s.",
		&uThisCoroutine(), uThisCoroutine().getName(), sourceName(), &source(), message() );
    } else {
	uAbort( "(uBaseCoroutine &)%p : Unhandled exception in coroutine %.256s raised non-locally from coroutine %.256s (%p), "
		"which was terminated due to a series of unhandled exceptions -- originally %s inside coroutine %.256s (%p).",
		&uThisCoroutine(), uThisCoroutine().getName(), sourceName(), &source(), message(), origName(), &origSource() );
    } // if
} // uBaseCoroutine::UnhandledException::defaultTerminate

void uBaseCoroutine::handleUnhandled( UnhandledException *event ) {
    _Resume uBaseCoroutine::UnhandledException( event->duplicate() ) _At resumer();
    notHalted = false;					// terminate coroutine
} // uBaseCoroutine::handleUnhandled

void uBaseCoroutine::handleUnhandled( uBaseEvent *event ) {
#   define uBaseCoroutineSuffixMsg1 "an unhandled thrown exception of type "
#   define uBaseCoroutineSuffixMsg2 "an unhandled resumed exception of type "
    char msg[sizeof(uBaseCoroutineSuffixMsg2) - 1 + uEHMMaxName]; // use larger message
    uBaseEvent::RaiseKind raisekind = event == nullptr ? uBaseEvent::ThrowRaise : event->getRaiseKind();
    strcpy( msg, raisekind == uBaseEvent::ThrowRaise ? uBaseCoroutineSuffixMsg1 : uBaseCoroutineSuffixMsg2 );
    uEHM::getCurrentEventName( raisekind, msg + strlen( msg ), uEHMMaxName );
    _Resume uBaseCoroutine::UnhandledException( event == nullptr ? event : event->duplicate(), msg ) _At resumer();
    notHalted = false;					// terminate coroutine
} // uBaseCoroutine::handleUnhandled


uBaseCoroutine::uCoroutineConstructor::uCoroutineConstructor( UPP::uAction f, UPP::uSerial &serial, uBaseCoroutine &coroutine, const char *name ) {
    if ( f == UPP::uYes ) {
	coroutine.startHere( (void (*)( uMachContext & ))uMachContext::invokeCoroutine );
	coroutine.name = name;
	coroutine.serial = &serial;			// set cormonitor's serial instance

#ifdef __U_PROFILER__
	if ( uThisTask().profileActive && uProfiler::uProfiler_registerCoroutine && // profiling & coroutine registered for profiling ?
	     dynamic_cast<uProcessorKernel *>(&coroutine) == nullptr ) { // and not kernel coroutine
	    (*uProfiler::uProfiler_registerCoroutine)( uProfiler::profilerInstance, coroutine, serial );
	} // if
#endif // __U_PROFILER__
    } // if
} // uBaseCoroutine::uCoroutineConstructor::uCoroutineConstructor


uBaseCoroutine::uCoroutineDestructor::uCoroutineDestructor( UPP::uAction f, uBaseCoroutine &coroutine ) : f( f ), coroutine( coroutine ) {
    // Clean up the stack of a non-terminated coroutine (i.e., run its destructors); a terminated coroutine's stack is
    // already cleaned up. Ignore the uProcessorKernel coroutine because it has a special shutdown sequence.

    // Because code executed during stack unwinding may access any coroutine data, unwinding MUST occur before running
    // the coroutine's destructor. A consequence of this semantics is that the destructor may not resume the coroutine,
    // so it is asymmetric with the coroutine's constructor.

    if ( coroutine.getState() != uBaseCoroutine::Halt	// coroutine not halted
	 && &coroutine.resumer() != nullptr		// and its main is started
	 && dynamic_cast<UPP::uProcessorKernel *>(&coroutine) == nullptr ) { // but not the processor Kernel
	// Mark for cancellation, then resume the coroutine to trigger a call to uPoll on the backside of its
	// suspend(). uPoll detects the cancellation and calls unwind_stack, which throws exception UnwindStack to
	// unwinding the stack. UnwindStack is ultimately caught inside uMachContext::uInvokeCoroutine.
	coroutine.cancel();
	coroutine.resume();
    } // if
} // uBaseCoroutine::uCoroutineDestructor::uCoroutineDestructor

#ifdef __U_PROFILER__
uBaseCoroutine::uCoroutineDestructor::~uCoroutineDestructor() {
    if ( f == uYes ) {
	if ( uThisTask().profileActive && uProfiler::uProfiler_deregisterCoroutine ) { // profiling this coroutine & coroutine registered for profiling ? 
	    (*uProfiler::uProfiler_deregisterCoroutine)( uProfiler::profilerInstance, coroutine );
	} // if
    } // if
} // uBaseCoroutine::uCoroutineDestructor::~uCoroutineDestructor
#endif // __U_PROFILER__

// Local Variables: //
// compile-command: "make install" //
// End: //
