//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// uMachContext.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Feb 25 15:46:42 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Aug 23 01:02:40 2012
// Update Count     : 719
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
#include <uC++.h>
#include <uAlign.h>
#ifdef __U_PROFILER__
#include <uProfiler.h>
#endif // __U_PROFILER__
#include <uHeapLmmm.h>

#include <uDebug.h>					// access: uDebugWrite
#undef __U_DEBUG_H__					// turn off debug prints

#include <cerrno>
#include <unistd.h>					// write

#if defined( __sparc__ ) || defined( __x86_64__ ) || defined( __ia64__ )
extern "C" void uInvokeStub( UPP::uMachContext * );
#endif

#if defined( __sparc__ )
#if defined(  __solaris__ )
#include <sys/stack.h>
#else
#include <machine/trap.h>
#include <machine/asm_linkage.h>
#include <machine/psl.h>
#endif // __solaris__
#endif // __sparc__


using namespace UPP;


extern "C" void pthread_deletespecific_( void * );	// see pthread simulation


#define MinStackSize 1000				// minimum feasible stack size in bytes


namespace UPP {
    void uMachContext::invokeCoroutine( uBaseCoroutine &This ) { // magically invoke the "main" of the most derived class
	// Called from the kernel when starting a coroutine or task so must switch back to user mode.

	This.setState( uBaseCoroutine::Active );	// set state of next coroutine to active
	THREAD_GETMEM( This )->enableInterrupts();

#ifdef __U_PROFILER__
	if ( uThisTask().profileActive && uProfiler::uProfiler_postallocateMetricMemory ) {
	    (*uProfiler::uProfiler_postallocateMetricMemory)( uProfiler::profilerInstance, uThisTask() );
	} // if
	// also appears in uBaseCoroutine::uContextSw2
	if ( ! THREAD_GETMEM( disableInt ) && uThisTask().profileActive && uProfiler::uProfiler_registerCoroutineUnblock ) {
	    (*uProfiler::uProfiler_registerCoroutineUnblock)( uProfiler::profilerInstance, uThisTask() );
	} // if
#endif // __U_PROFILER__

	// At this point, execution is on the stack of the new coroutine or task that has just been switched to by the
	// kernel.  Therefore, interrupts can legitimately occur now.

	try {
	    This.corStarter();				// moved from uCoroutineMain to allow recursion on "main"
	    uBaseCoroutine::asyncpoll();		// cancellation checkpoint
	    This.main();				// start coroutine's "main" routine
	} catch ( uBaseCoroutine::UnwindStack &u ) {
	    u.exec_dtor = false;			// defuse the bomb or otherwise unwinding will continue
	    This.notHalted = false;			// terminate coroutine
	} catch( uBaseCoroutine::UnhandledException &ex ) {
	    This.handleUnhandled( &ex );
	} catch ( uBaseEvent &ex ) {
	    This.handleUnhandled( &ex );
	} catch( ... ) {				// unknown exception ?
	    This.handleUnhandled();
	} // try

	// check outside handler so exception is freed before suspending
	if ( ! This.notHalted ) {			// exceptional ending ?
	    This.suspend();				// restart last resumer, which should immediately propagate the nonlocal exception
	} // if
	if ( &This != activeProcessorKernel ) {		// uProcessorKernel exit ?
	    This.corFinish();
	    uAbort( "internal error, uMachContext::invokeCoroutine, no return" );
	} // if
    } // uMachContext::invokeCoroutine


    void uMachContext::invokeTask( uBaseTask &This ) {	// magically invoke the "main" of the most derived class
	// Called from the kernel when starting a coroutine or task so must switch back to user mode.

#if defined(__U_MULTI__)
	assert( THREAD_GETMEM( activeTask ) == &This );
#endif

	errno = 0;					// reset errno for each task
	This.currCoroutine->setState( uBaseCoroutine::Active ); // set state of next coroutine to active
	This.setState( uBaseTask::Running );

	// At this point, execution is on the stack of the new coroutine or task that has just been switched to by the
	// kernel.  Therefore, interrupts can legitimately occur now.
	THREAD_GETMEM( This )->enableInterrupts();

	uHeapControl::startTask();

#ifdef __U_PROFILER__
	if ( uThisTask().profileActive && uProfiler::uProfiler_postallocateMetricMemory ) {
	    (*uProfiler::uProfiler_postallocateMetricMemory)( uProfiler::profilerInstance, uThisTask() );
	} // if
#endif // __U_PROFILER__

	uPthreadable *pthreadable = dynamic_cast< uPthreadable * > (&This);

	try {
	    try {
		uBaseCoroutine::asyncpoll();		// cancellation checkpoint
		This.main();				// start task's "main" routine
	    } catch( uBaseCoroutine::UnwindStack &evt ) {
		evt.exec_dtor = false;			// defuse the unwinder	
	    } catch( uBaseEvent &ex ) {
		ex.defaultTerminate();
		cleanup( This );			// preserve current exception
	    } catch( ... ) {
		if ( ! This.cancelInProgress() ) {
		    cleanup( This );			// preserve current exception
		    // CONTROL NEVER REACHES HERE!
		} // if
		if ( pthreadable ) {
		    pthreadable->stop_unwinding = true;	// prevent continuation of unwinding
		} // if
	    } // try
	} catch (...) {
	    uEHM::terminate();				// if defaultTerminate or std::terminate throws exception
	} // try
	// NOTE: this code needs further protection as soon as asynchronous cancellation is supported

	// Clean up storage associated with the task for pthread thread-specific data, e.g., exception handling
	// associates thread-specific data with any task.

	if ( This.pthreadData != NULL ) {
	    pthread_deletespecific_( This.pthreadData );
	} // if

	uHeapControl::finishTask();

	This.notHalted = false;
	This.setState( uBaseTask::Terminate );

	This.getSerial().leave2();
	// CONTROL NEVER REACHES HERE!
	uAbort( "(uMachContext &)%p.invokeTask() : internal error, attempt to return.", &This );
    } // uMachContext::invokeTask


    void uMachContext::cleanup( uBaseTask &This ) {
	try {
	    std::terminate();				// call task terminate routine
	} catch( ... ) {				// control should not return
	} // try

	if ( This.pthreadData != NULL ) {		// see above for explanation
	    pthread_deletespecific_( This.pthreadData );
	} // if

	uEHM::terminate();				// call abort terminate routine
    } // uMachContext::cleanup


    void uMachContext::extraSave() {
	// Don't need to test extras bit, it is sufficent to check context list

	uContext *context;
	for ( uSeqIter<uContext> iter(additionalContexts); iter >> context; ) {
	    context->save();
	} // for
    } // uMachContext::extraSave


    void uMachContext::extraRestore() {
	// Don't need to test extras bit, it is sufficent to check context list

	uContext *context;
	for ( uSeqIter<uContext> iter(additionalContexts); iter >> context; ) {
	    context->restore();
	} // for
    } // uMachContext::extraRestore


    void uMachContext::startHere( void (*uInvoke)( uMachContext & ) ) {
#if defined( __i386__ )

#if ! defined( __U_SWAPCONTEXT__ )
	struct FakeStack {
	    void *fixedRegisters[3];			// fixed registers ebx, edi, esi (popped on 1st uSwitch, values unimportant)
	    void *rturn;				// where to go on return from uSwitch
	    void *dummyReturn;				// fake return compiler would have pushed on call to uInvoke
	    void *argument;				// for 16-byte ABI, 16-byte alignment starts here
	    void *padding[3];				// padding to force 16-byte alignment, as "base" is 16-byte aligned
	}; // FakeStack

	((uContext_t *)context)->SP = (char *)base - sizeof( FakeStack );
	((uContext_t *)context)->FP = NULL;		// terminate stack with NULL fp

	((FakeStack *)(((uContext_t *)context)->SP))->dummyReturn = NULL;
	((FakeStack *)(((uContext_t *)context)->SP))->argument = this; // argument to uInvoke
	((FakeStack *)(((uContext_t *)context)->SP))->rturn = rtnAdr( (void (*)())uInvoke );
#else
	if ( ::getcontext( (ucontext *)context ) == -1 ) { // initialize ucontext area
	    uAbort( "internal error, getcontext failed" );
	} // if
	((ucontext *)context)->uc_stack.ss_sp = (char *)limit;
	((ucontext *)context)->uc_stack.ss_size = size; 
	((ucontext *)context)->uc_stack.ss_flags = 0;
	::makecontext( (ucontext *)context, (void (*)())uInvoke, 2, this );
	// TEMPORARY: stack is incorrectly initialized to allow stack walking
	((ucontext *)context)->uc_mcontext.gregs[ REG_EBP ] = 0; // terminate stack with NULL fp
#endif

#elif defined( __x86_64__ )

#if ! defined( __U_SWAPCONTEXT__ )
	struct FakeStack {
	    void *fixedRegisters[5];			// fixed registers rbx, r12, r13, r14, r15
	    void *rturn;				// where to go on return from uSwitch
	    void *dummyReturn;				// NULL return address to provide proper alignment
	}; // FakeStack

	((uContext_t *)context)->SP = (char *)base - sizeof( FakeStack );
	((uContext_t *)context)->FP = NULL;		// terminate stack with NULL fp

	((FakeStack *)(((uContext_t *)context)->SP))->dummyReturn = NULL;
	((FakeStack *)(((uContext_t *)context)->SP))->rturn = rtnAdr( (void (*)())uInvokeStub );
	((FakeStack *)(((uContext_t *)context)->SP))->fixedRegisters[0] = this;
	((FakeStack *)(((uContext_t *)context)->SP))->fixedRegisters[1] = rtnAdr( (void (*)())uInvoke );
#else
	// makecontext is difficult to support.  See http://sources.redhat.com/bugzilla/show_bug.cgi?id=404
	#error uC++ : internal error, swapcontext cannot be used on the x86_64 architecture
#endif

#elif defined( __ia64__ )

#if ! defined( __U_SWAPCONTEXT__ )

	struct FakeStack {
	    void *scratch1[2];				// abi-mandated scratch area
	    struct preservedState {
		void *b5;
		void *b4;
		void *b3;
		void *b2;
		void *b1;
		void *b0;
		void *pr;
		void *lc;
		void *pfs;
		void *fpsr;
		void *unat;
		void *spill_unat;
		void *rnat;
		void *r7;
		void *r6;
		void *r5;
		void *r4;
		void *r1;
	    } preserved;
	    void *scratch2[2];				// abi-mandated scratch area
	}; // FakeStack

	((uContext_t *)context)->SP = (char *)base - sizeof( FakeStack );
	((uContext_t *)context)->BSP = (char*)limit + 16;
	sigemptyset( &((uContext_t *)context)->sigMask );
#ifdef __U_MULTI__
	sigaddset( &((uContext_t *)context)->sigMask, SIGALRM );
#endif // __U_MULTI__
	memset( ((uContext_t *)context)->SP, 0, sizeof( FakeStack ) );
	((FakeStack *)(((uContext_t *)context)->SP))->preserved.b0 = rtnAdr( (void (*)())uInvokeStub );
	((FakeStack *)(((uContext_t *)context)->SP))->preserved.r1 = gpAdr( (void (*)())uInvokeStub );
	asm ( "mov.m %0 = ar.fpsr" : "=r" (((FakeStack *)(((uContext_t *)context)->SP))->preserved.fpsr) );
	((FakeStack *)(((uContext_t *)context)->SP))->preserved.r4 = this;
	((FakeStack *)(((uContext_t *)context)->SP))->preserved.b1 = rtnAdr( (void (*)())uInvoke );
	((FakeStack *)(((uContext_t *)context)->SP))->preserved.b2 = NULL; // null terminate for stack walking
#else
	if ( ::getcontext( (ucontext *)context ) == -1 ) { // initialize ucontext area
	    uAbort( "internal error, getcontext failed" );
	} // if
	((ucontext *)context)->uc_stack.ss_sp = (char *)limit;
	((ucontext *)context)->uc_stack.ss_size = size;
	((ucontext *)context)->uc_stack.ss_flags = 0;
	::makecontext( (ucontext *)context, (void (*)())uInvoke, 2, this );
#endif

#elif defined( __sparc__ )

#if ! defined( __U_SWAPCONTEXT__ )
	struct FakeStack {
	    void *localRegs[8];
	    void *inRegs[8];
#if __U_WORDSIZE__ == 32
	    void *structRetAddress;
#endif // __U_WORDSIZE__ == 32
	    void *calleeRegArgs[6];
	}; // FakeStack

	((uContext_t *)context)->FP = (char *)(SA( (unsigned long)base - STACK_ALIGN + 1 ) - SA( MINFRAME ) - STACK_BIAS );
	((uContext_t *)context)->SP = (char *)((uContext_t *)context)->FP - SA( MINFRAME );
	((uContext_t *)context)->PC = (char *)rtnAdr( (void (*)())uInvokeStub ) - 8;

	((FakeStack *)((char *)((uContext_t *)context)->FP + STACK_BIAS))->inRegs[0] = this; // argument to uInvoke
	((FakeStack *)((char *)((uContext_t *)context)->FP + STACK_BIAS))->inRegs[1] = (void *)uInvoke; // uInvoke routine
	((FakeStack *)((char *)((uContext_t *)context)->FP + STACK_BIAS))->inRegs[6] = NULL; // terminate stack with NULL fp
#else
	if ( ::getcontext( (ucontext *)context ) == -1 ) { // initialize ucontext area
	    uAbort( " : internal error, getcontext failed" );
	} // if
	((ucontext *)context)->uc_stack.ss_sp = (char *)base - 8;	// TEMPORARY: -8 for bug in Solaris (fixed in Solaris 4.10)
	((ucontext *)context)->uc_stack.ss_size = size; 
	((ucontext *)context)->uc_stack.ss_flags = 0;
	::makecontext( (ucontext *)context, (void (*)(...))uInvoke, 2, this );
	// TEMPORARY: stack is incorrectly initialized to allow stack walking
	*((int *)base - 8) = 0;				// terminate stack with NULL fp
#endif

#else
	#error uC++ : internal error, unsupported architecture
#endif
    } // uMachContext::startHere


    /**************************************************************
	,-----------------. \
	|                 | |
	| __U_CONTEXT_T__ | } (multiple of 8)
	|                 | |
	`-----------------' / <--- context (16 byte align)
	,-----------------. \ <--- base (stack grows down)
	|                 | |
	|    task stack   | } size (multiple of 16)
	|                 | |
	`-----------------' / <--- limit (16 byte align)
	0/8                   <--- storage
	,-----------------.
	|   guard page    |   debug only
	| write protected |
	`-----------------'   <--- 4/8/16K alignment
    **************************************************************/

    void uMachContext::createContext( unsigned int storageSize ) { // used by all constructors
	size_t cxtSize = uCeiling( sizeof(__U_CONTEXT_T__), 8 ); // minimum alignment

	if ( storage == NULL ) {
	    userStack = false;
	    size = uCeiling( storageSize, 16 );
	    // use malloc/memalign because "new" raises an exception for out-of-memory
#ifdef __U_DEBUG__
	    storage = memalign( pageSize, cxtSize + size + pageSize );
	    if ( ::mprotect( storage, pageSize, PROT_NONE ) == -1 ) {
		uAbort( "(uMachContext &)%p.createContext() : internal error, mprotect failure, error(%d) %s.", this, errno, strerror( errno ) );
	    } // if
#else
	    // assume malloc has 8 byte alignment so add 8 to allow rounding up to 16 byte alignment
	    storage = malloc( cxtSize + size + 8 );
#endif // __U_DEBUG__
	    if ( storage == NULL ) {
		uAbort( "Attempt to allocate %d bytes of storage for coroutine or task execution-state but insufficient memory available.", size );
	    } // if
#ifdef __U_DEBUG__
	    limit = (char *)storage + pageSize;
#else
	    limit = (char *)uCeiling( (unsigned long)storage, 16 ); // minimum alignment
#endif // __U_DEBUG__
	} else {
#ifdef __U_DEBUG__
	    if ( ((size_t)storage & (uAlign() - 1)) != 0 ) { // multiple of uAlign ?
		uAbort( "Stack storage %p for task/coroutine must be aligned on %d byte boundary.", storage, (int)uAlign() );
	    } // if
#endif // __U_DEBUG__
	    userStack = true;
	    size = storageSize - cxtSize;
	    if ( size % 16 != 0 ) size -= 8;
	    limit = (char *)uCeiling( (unsigned long)storage, 16 ); // minimum alignment
	} // if
#ifdef __U_DEBUG__
	if ( size < MinStackSize ) {			// below minimum stack size ?
	    uAbort( "Stack size %d provides less than minimum of %d bytes for a stack.", size, MinStackSize );
	} // if
#endif // __U_DEBUG__

	base = (char *)limit + size;
	context = base;
	top = (char *)context + cxtSize;

	extras.allExtras = 0;
    } // uMachContext::createContext


    void *uMachContext::stackPointer() const {
	if ( &uThisCoroutine() == this ) {		// accessing myself ?
	    void *sp;					// use my current stack value
#if defined( __i386__ )
	    asm( "movl %%esp,%0" : "=m" (sp) : );
#elif defined( __x86_64__ )
	    asm( "movq %%rsp,%0" : "=m" (sp) : );
#elif defined( __ia64__ )
	    asm( "mov %0 = r12" : "=r" (sp) : );
#elif defined( __sparc__ )
	    asm( "mov %%sp,%0" : "=r" (sp) : );
	    sp = (void*)((char*)sp + STACK_BIAS);
#else
	    #error uC++ : internal error, unsupported architecture
#endif
	    return sp;
	} else {					// accessing another coroutine

#if ! defined( __U_SWAPCONTEXT__ )

#if defined( __sparc__ )
	    return (void*)((char*)((uContext_t *)context)->SP + STACK_BIAS);
#else
	    return ((uContext_t *)context)->SP;
#endif // __sparc__

#else  // __U_SWAPCONTEXT__
	    return (void *)(((ucontext_t *)context)->uc_mcontext.
#if defined( __linux__ )

#if defined( __i386__ )
		    gregs[REG_ESP]);
#elif defined( __x86_64__ )
		    gregs[REG_RSP]);
#elif defined( __ia64__ )
		    sc_gr[12]);
#else
		    #error uC++ : internal error, unsupported architecture
#endif // architectures

#elif defined( __freebsd__ )

#if defined( __i386__ )
		    mc_esp;
#elif defined( __x86_64__ )
		    gregs[REG_RSP]);
#else
		    #error uC++ : internal error, unsupported architecture
#endif // architectures

#elif defined( __solaris__ )

#if defined( __sparc__ )
		    (void*)((char*)gregs[REG_SP] + STACK_BIAS));
#else
		    #error uC++ : internal error, unsupported architecture
#endif // architectures

#endif // operating system

#endif // __U_SWAPCONTEXT__
	} // if
    } // uMachContext::stackPointer


#if defined( __ia64__ )
    void *uMachContext::registerStackPointer() const {
#if defined( __U_SWAPCONTEXT__ )
	if ( &uThisCoroutine() == this ) {		// accessing myself ?
	    void *bsp;					// use my current stack value
	    asm( "mov %0 = ar.bsp" : "=r" (bsp) : );
	    return bsp;
	} else {					// accessing another coroutine
	    return (void *)(((ucontext_t *)context)->uc_mcontext.sc_ar_bsp);
	} // if
#else
	return ((uContext_t *)context)->BSP;
#endif
    } // uMachContext::registerStackPointer
#endif // __ia64__


    ptrdiff_t uMachContext::stackFree() const {
	return (char *)stackPointer() - (char *)limit;
    } // uMachContext::stackFree


    ptrdiff_t uMachContext::stackUsed() const {
	return (char *)base - (char *)stackPointer();
    } // uMachContext::stackUsed


    void uMachContext::verify() {
	// Ignore boot task as it uses the UNIX stack.
	if ( storage == ((uBaseTask *)uKernelModule::bootTask)->storage ) return;

	void *sp = stackPointer();			// optimization

	if ( sp < limit ) {
	    uAbort( "Stack overflow detected: stack pointer %p below limit %p.\n"
		    "Possible cause is allocation of large stack frame(s) and/or deep call stack.",
		    sp, limit );
	} else if ( stackFree() < 4 * 1024 ) {
	    // Do not use fprintf because it uses a lot of stack space.
#define WARNING4K "uC++ Runtime warning : within 4K of stack limit.\n"	    
	    uDebugWrite( STDERR_FILENO, WARNING4K, sizeof(WARNING4K) - 1 );
	} else if ( sp > base ) {
	    uAbort( "Stack underflow detected: stack pointer %p above base %p.\n"
		    "Possible cause is corrupted stack frame via overwriting memory.",
		    sp, base );
#if defined( __ia64__ )
	} else if ( registerStackPointer() >= sp ) {
	    // on ia64 the stack grows from both ends; when the two stack pointers cross, we have overflow
	    uAbort( "Stack overflow detected: stack pointer %p at or below register stack pointer %p.\n"
		    "Possible cause is allocation of large stack frame(s) and/or deep call stack.",
		    sp, registerStackPointer() );
#endif // __ia64__
	} // if
    } // uMachContext::verify


    void *uMachContext::rtnAdr( void (*rtn)() ) {
#if defined( __linux__ ) && defined( __ia64__ )
	if ( ! rtn ) return NULL;

	struct TOC {
	    void *rtn;
	    void *toc;
	};

	return ((TOC *)rtn)->rtn;
#else
	return (void *)rtn;
#endif
    } // uMachContext::rtnAdr


#if defined( __linux__ ) && defined( __ia64__ )
    void *uMachContext::gpAdr( void (*rtn)() ) {
	if ( ! rtn ) return NULL;

	struct TOC {
	    void *rtn;
	    void *toc;
	};

	return ((TOC *)rtn)->toc;
    } // uMachContext::gpAdr
#endif
} // UPP


// Local Variables: //
// compile-command: "make install" //
// End: //
