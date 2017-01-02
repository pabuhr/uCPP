//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1996
// 
// uBaseTask.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Jan  8 16:14:20 1996
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Dec 29 12:18:27 2016
// Update Count     : 303
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


using namespace UPP;


//######################### uBaseTask #########################


void uBaseTask::createTask( uCluster &cluster ) {
#ifdef __U_DEBUG__
    currSerialOwner = this;
    currSerialCount = 1;
    currSerialLevel = 0;
#endif // __U_DEBUG__
    state = Start;
    recursion = mutexRecursion = 0;
    currCluster = &cluster;				// remember the cluster task is created on
    currCoroutine = this;				// the first coroutine that a task executes is itself
    acceptedCall = nullptr;				// no accepted mutex entry yet
    priority = activePriority = 0;
    inheritTask = this;

    // exception handling

    terminateRtn = uEHM::terminate;			// initialize default terminate routine

#ifdef __U_PROFILER__
    // profiling

    profileActive = false;				// can be read before uTaskConstructor is called
#endif // __U_PROFILER__

    // debugging
#if __U_LOCALDEBUGGER_H__
    DebugPCandSRR = nullptr;
    uProcessBP = false;					// used to prevent triggering breakpoint while processing one
#endif // __U_LOCALDEBUGGER_H__

    // pthreads

    pthreadData = nullptr;

    // memory allocation

    if ( this != (uBaseTask *)uKernelModule::bootTask ) {
	heapData = nullptr;
	uHeapControl::prepareTask( this );
    } // if
} // uBaseTask::createTask


uBaseTask::uBaseTask( uCluster &cluster, uProcessor &processor ) : uBaseCoroutine( cluster.getStackSize() ), clusterRef( *this ), readyRef( *this ), entryRef( *this ), mutexRef( *this ), bound( processor ) {
    createTask( cluster );
} // uBaseTask::uBaseTask


void uBaseTask::setState( uBaseTask::State s ) {
    state = s;

#ifdef __U_PROFILER__
    if ( profileActive && uProfiler::uProfiler_registerTaskExecState ) { 
	(*uProfiler::uProfiler_registerTaskExecState)( uProfiler::profilerInstance, *this, state ); 
    } // if
#endif // __U_PROFILER__
} // uBaseTask::setState


uBaseTask::uTaskConstructor::uTaskConstructor( uAction f, uSerial &serial, uBaseTask &task, uBasePIQ &piq, const char *n, bool profile ) : f( f ), serial( serial ), task( task ) {
#ifdef __U_DEBUG_H__
    uDebugPrt( "(uTaskConstructor &)%p.uTaskConstructor, f:%d, s:%p, t:%p, piq:%p, n:%s, profile:%d\n", this, f, &serial, &task, &piq, n, profile );
#endif // __U_DEBUG_H__
    if ( f == uYes ) {
	task.startHere( (void (*)( uMachContext & ))uMachContext::invokeTask );
	task.name = n;
	task.serial = &serial;			// set task's serial instance
	task.setSerial( serial );
	task.uPIQ = &piq;

#ifdef __U_PROFILER__
	task.profileActive = profile;

	if ( task.profileActive && uProfiler::uProfiler_registerTask ) { // profiling this task & task registered for profiling ? 
	    (*uProfiler::uProfiler_registerTask)( uProfiler::profilerInstance, task, serial, uThisTask() );
	} // if
#endif // __U_PROFILER__

	serial.acceptSignalled.add( &(task.mutexRef) );

#if __U_LOCALDEBUGGER_H__
	if ( uLocalDebugger::uLocalDebuggerActive ) uLocalDebugger::uLocalDebuggerInstance->checkPoint();
#endif // __U_LOCALDEBUGGER_H__

	task.currCluster->taskAdd( task );		// add task to the list of tasks on this cluster
    } // if
} // uBaseTask::uTaskConstructor::uTaskConstructor


uBaseTask::uTaskConstructor::~uTaskConstructor() {
    if ( f == uYes && std::uncaught_exception() ) {
	// An exception was thrown during task construction. It is necessary to clean up constructor side-effects.

	// Since no task could have been accepted, the acceptor/signaller stack should only contain this task.  It
	// must be removed here so that it is not scheduled by ~uSerial.
	serial.acceptSignalled.drop();

	uTaskDestructor::cleanup( task );
    } // if
} // uBaseTask::uTaskConstructor::~uTaskConstructor


uBaseTask::uTaskDestructor::~uTaskDestructor() {
    if ( f == uYes ) {
#ifdef __U_DEBUG__
	if ( task.uBaseCoroutine::getState() != uBaseCoroutine::Halt ) {
	    uAbort( "Attempt to delete task %.256s (%p) that is not halted.\n"
		    "Possible cause is task blocked on a condition queue.",
		    task.getName(), &task );
	} // if
#endif // __U_DEBUG__

	cleanup( task );
    } // if
} // uBaseTask::uTaskDestructor::uTaskDestructor


void uBaseTask::uTaskDestructor::cleanup( uBaseTask &task ) {
#if __U_LOCALDEBUGGER_H__
    if ( uLocalDebugger::uLocalDebuggerActive ) uLocalDebugger::uLocalDebuggerInstance->checkPoint();
#endif // __U_LOCALDEBUGGER_H__

#ifdef __U_PROFILER__
    task.profileActive = false;
    if ( task.profileTaskSamplerInstance && uProfiler::uProfiler_deregisterTask ) { // task registered for profiling ?
	(*uProfiler::uProfiler_deregisterTask)( uProfiler::profilerInstance, task );
    } // if
#endif // __U_PROFILER__

    task.currCluster->taskRemove( task );		// remove the task from the list of tasks that live on this cluster.
} // uBaseTask::uTaskDestructor::cleanup


uBaseTask::uTaskMain::uTaskMain( uBaseTask &task ) : task( task ) {
    // SKULLDUGGERY: To allow "main" to be treated as a normal member routine, a counter is used to allow recursive
    // entry.

    task.recursion += 1;
    if ( task.recursion == 1 ) {			// first call ?
#ifdef __U_PROFILER__
	if ( task.profileActive && uProfiler::uProfiler_registerTaskStartExecution ) { 
	    (*uProfiler::uProfiler_registerTaskStartExecution)( uProfiler::profilerInstance, task ); 
	} // if
#endif // __U_PROFILER__

#if __U_LOCALDEBUGGER_H__
	// Registering a task with the global debugger must occur in this routine for the register set to be
	// correct.

	if ( uLocalDebugger::uLocalDebuggerActive ) {
	    uLocalDebugger::uLocalDebuggerInstance->createULThread();
	} // if
#endif // __U_LOCALDEBUGGER_H__
    } // if
} // uBaseTask::uTaskMain::uTaskMain


uBaseTask::uTaskMain::~uTaskMain() {
    task.recursion -= 1;
    if ( task.recursion == 0 ) {
#ifdef __U_PROFILER__
	if ( task.profileActive && uProfiler::uProfiler_registerTaskEndExecution ) {
	    (*uProfiler::uProfiler_registerTaskEndExecution)( uProfiler::profilerInstance, task ); 
	} // if
#endif // __U_PROFILER__

#if __U_LOCALDEBUGGER_H__
	if ( uLocalDebugger::uLocalDebuggerActive ) uLocalDebugger::uLocalDebuggerInstance->destroyULThread();
#endif // __U_LOCALDEBUGGER_H__
    } // if
} // uBaseTask::uTaskMain::~uTaskMain


void uBaseTask::wake() {
    setState( Ready );					// task is marked available for execution
    currCluster->makeTaskReady( *this );		// put the task on the ready queue of the cluster
} // uBaseTask::wake


uBaseTask::uBaseTask( uCluster &cluster ) : uBaseCoroutine( cluster.getStackSize() ), clusterRef( *this ), readyRef( *this ), entryRef( *this ), mutexRef( *this ), bound( *(uProcessor *)0 ) {
    createTask( cluster );
} // uBaseTask::uBaseTask


uCluster &uBaseTask::migrate( uCluster &cluster ) {
#ifdef __U_DEBUG_H__
    uDebugPrt( "(uBaseTask &)%p.migrate, from cluster:%p to cluster:%p\n", &uThisTask(), &uThisCluster(), &cluster );
#endif // __U_DEBUG_H__

    uBaseTask &task = uThisTask();			// optimization
    assert( &task.bound == nullptr );

    // A simple optimization: migrating to the same cluster that the task is currently executing on simply returns the
    // value of the current cluster.  Therefore, migrate does not always produce a context switch.

  if ( &cluster == task.currCluster ) return cluster;

#if defined( __U_DEBUG__ ) && defined( __U_MULTI__ )
    volatile uKernelModule::uKernelModuleData *before = THREAD_GETMEM( This );
#endif // __U_DEBUG__ && __U_MULTI__

#if __U_LOCALDEBUGGER_H__
    if ( uLocalDebugger::uLocalDebuggerActive ) uLocalDebugger::uLocalDebuggerInstance->checkPoint();
#endif // __U_LOCALDEBUGGER_H__

    // Remove the task from the list of tasks that live on this cluster, and add it to the list of tasks that live on
    // the new cluster.

    uCluster &prevCluster = *task.currCluster;		// save for return

#ifdef __U_PROFILER__
    if ( task.profileActive && uProfiler::uProfiler_registerTaskMigrate ) { // task registered for profiling ?
	(*uProfiler::uProfiler_registerTaskMigrate)( uProfiler::profilerInstance, task, prevCluster, cluster );
    } // if
#endif // __U_PROFILER__

    // Interrupts are disabled because once the task is removed from a cluster it is dangerous for it to be placed back
    // on that cluster during an interrupt.  Therefore, interrupts are disabled until the task is on its new cluster.

    THREAD_GETMEM( This )->disableInterrupts();

    prevCluster.taskRemove( task );			// remove from current cluster
    task.currCluster = &cluster;			// change task's notion of which cluster it is executing on
    cluster.taskAdd( task );				// add to new cluster

    THREAD_GETMEM( This )->enableInterrupts();

#if __U_LOCALDEBUGGER_H__
    if ( uLocalDebugger::uLocalDebuggerActive ) uLocalDebugger::uLocalDebuggerInstance->migrateULThread( cluster );
#endif // __U_LOCALDEBUGGER_H__

    // swapcontext saves and restores the signal mask while uC++ context switch does not.
#if defined( __U_MULTI__ ) && defined( __U_SWAPCONTEXT__ )
    // when stepping off the system cluster, the SIGALRM must be reset to blocked
    if ( &prevCluster == uKernelModule::systemCluster ) {
 	sigset_t new_mask;
 	sigemptyset( &new_mask );
 	sigaddset( &new_mask, SIGALRM );
 	if ( sigprocmask( SIG_BLOCK, &new_mask, nullptr ) == -1 ) {
 	    uAbort( "internal error, sigprocmask" );
 	} // if
    } // if
#endif // __U_MULTI__ && __U_SWAPCONTEXT__

    // Force a context switch so the task is scheduled on the new cluster.

    yield();

#if defined( __U_MULTI__ ) && defined( __U_SWAPCONTEXT__ )
    // when stepping onto the system cluster, the SIGALRM must reset to unblocked
    if ( &cluster == uKernelModule::systemCluster ) {
	sigset_t new_mask;
	sigemptyset( &new_mask );
	sigaddset( &new_mask, SIGALRM );
	if ( sigprocmask( SIG_UNBLOCK, &new_mask, nullptr ) == -1 ) {
	    uAbort( "internal error, sigprocmask" );
	} // if
    } // if
#endif // __U_MULTI__ && __U_SWAPCONTEXT__

#if defined( __U_DEBUG__ ) && defined( __U_MULTI__ )
    assert( before != THREAD_GETMEM( This ) );
#endif // __U_DEBUG__ && __U_MULTI__

    return prevCluster;					// return reference to previous cluster
} // uBaseTask::migrate


void uBaseTask::uYieldYield( unsigned int times ) {	// inserted by translator for -yield
    // Calls to uYieldYield can be inserted in any inlined routine, which can than be called from a uWhen clause,
    // resulting in an attempt to context switch while holding a spin lock. To ensure assert checking in normal usages
    // of yield, this check cannot be inserted in yield.

    if ( ! THREAD_GETMEM( disableIntSpin ) ) {
	for ( ; times > 0 ; times -= 1 ) {
	    uYieldNoPoll();
	} // for
    } // if
} // uBaseTask::uYieldYield


void uBaseTask::uYieldInvoluntary() {
    assert( ! THREAD_GETMEM( disableIntSpin ) );
//#ifdef __U_DEBUG__
//    if ( this != &uThisTask() ) {
//	uAbort( "Attempt to yield the execution of task %.256s (%p) by task %.256s (%p).\n"
//		"A task may only yield itself.",
//		getName(), this, uThisTask().getName(), &uThisTask() );
//    } // if
//#endif // __U_DEBUG__

#ifdef __U_PROFILER__
    // Are the uC++ kernel memory allocation hooks active?
    if ( profileActive && uProfiler::uProfiler_preallocateMetricMemory ) {
	// create a preallocated memory array on the stack
	void *ptrs[U_MAX_METRICS];

	(*uProfiler::uProfiler_preallocateMetricMemory)( uProfiler::profilerInstance, ptrs, *this );

	THREAD_GETMEM( This )->disableInterrupts();
	(*uProfiler::uProfiler_setMetricMemoryPointers)( uProfiler::profilerInstance, ptrs, *this ); // force task to use local memory array
	activeProcessorKernel->scheduleInternal( this ); // find someone else to execute; wake on kernel stack
	(*uProfiler::uProfiler_resetMetricMemoryPointers)( uProfiler::profilerInstance, *this );     // reset task to use its native memory array
	THREAD_GETMEM( This )->enableInterrupts();

	// free any blocks of memory not used by metrics
	for ( int metric = 0; metric < uProfiler::profilerInstance->numMemoryMetrics; metric += 1 ) {
	    free( ptrs[metric] );
	} // for
    } else {
#endif // __U_PROFILER__
	THREAD_GETMEM( This )->disableInterrupts();
	activeProcessorKernel->scheduleInternal( this ); // find someone else to execute; wake on kernel stack
	THREAD_GETMEM( This )->enableInterrupts();
#ifdef __U_PROFILER__
    } // if
#endif // __U_PROFILER__
} // uBaseTask::uYieldInvoluntary


void uBaseTask::uSleep( uTime time ) {
#ifdef __U_DEBUG__
    if ( this != &uThisTask() ) {
	uAbort( "Attempt to put task %.256s (%p) to sleep by task %.256s (%p).\n"
		"A task may only put itself to sleep.",
		getName(), this, uThisTask().getName(), &uThisTask() );
    } // if
#endif // __U_DEBUG__

  if ( time <= activeProcessorKernel->kernelClock.getTime() ) return;

    uWakeupHndlr handler( *this );			// handler to wake up blocking task
    uEventNode uRTEvent( *this, handler, time );	// event node for event list
    uRTEvent.add( true );				// block until time expires
} // uBaseTask::uSleep


void uBaseTask::uSleep( uDuration duration ) {
    uSleep( activeProcessorKernel->kernelClock.getTime() + duration );
} // uBaseTask::uSleep


#ifdef __U_PROFILER__
void uBaseTask::profileActivate( uBaseTask &task ) {
    if ( ! profileTaskSamplerInstance ) {		// already registered for profiling ?
	if ( uProfiler::uProfiler_registerTask ) {	// task registered for profiling ? 
	    (*uProfiler::uProfiler_registerTask)( uProfiler::profilerInstance, *this, *serial, task );
	    profileActive = true;
	} // if
    } else {
	profileActive = true;
    } // if 
} // uBaseTask::profileActivate


void uBaseTask::profileActivate() {
    profileActivate( *(uBaseTask *)0 );
} // uBaseTask::profileActivate


void uBaseTask::profileInactivate() {
    if ( profileActive && uProfiler::uProfiler_profileInactivate ) {
	(*uProfiler::uProfiler_profileInactivate)( uProfiler::profilerInstance, *this );
    } // if
    profileActive = false;
} // uBaseTask::profileInactivate


void uBaseTask::printCallStack() const {
    if ( profileTaskSamplerInstance ) {
	(*uProfiler::uProfiler_printCallStack)( profileTaskSamplerInstance );
    } // if
} // uBaseTask::printCallStack
#endif // __U_PROFILER__


// Local Variables: //
// compile-command: "make install" //
// End: //
