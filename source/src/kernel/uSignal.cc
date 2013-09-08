//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uSignal.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sun Dec 19 16:32:13 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Nov 13 15:10:22 2012
// Update Count     : 783
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
#include <uHeapLmmm.h>
#include <uDebug.h>					// access: uDebugWrite
#undef __U_DEBUG_H__					// turn off debug prints

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>					// _exit
#include <sys/wait.h>
#include <sys/types.h>
#include <ucontext.h>


namespace UPP {
    sigset_t uSigHandlerModule::block_mask;

    void uSigHandlerModule::signal( int sig, void (*handler)(__U_SIGPARMS__), int flags ) { // name clash with uSignal statement
	struct sigaction act;

	act.sa_sigaction = (void (*)(int, siginfo_t *, void *))handler;
	sigemptyset( &act.sa_mask );
	sigaddset( &act.sa_mask, SIGALRM );		// disabled during signal handler
	sigaddset( &act.sa_mask, SIGUSR1 );

#ifdef __U_PROFILER__
	sigaddset( &act.sa_mask, SIGVTALRM );
#if defined( __U_HW_OVFL_SIG__ )
	sigaddset( &act.sa_mask, __U_HW_OVFL_SIG__ );
#endif // __U_HW_OVFL_SIG__
#endif // __U_PROFILER__

	act.sa_flags = flags;

	if ( sigaction( sig, &act, NULL ) == -1 ) {
	    // THE KERNEL IS NOT STARTED SO CALL NO uC++ ROUTINES!
	    char helpText[256];
	    int len = snprintf( helpText, 256, " uSigHandlerModule::signal( sig:%d, handler:%p, flags:%d ), problem installing signal handler, error(%d) %s.\n",
				sig, handler, flags, errno, strerror( errno ) );
	    uDebugWrite( STDERR_FILENO, helpText, len );
	    _exit( EXIT_FAILURE );
	} // if
    } // uSigHandlerModule::signal


    void *uSigHandlerModule::signalContextPC( __U_SIGCXT__ cxt ) {
#if defined( __i386__ )
#if defined( __linux__ )
	return (void *)(cxt->uc_mcontext.gregs[REG_EIP]);
#elif defined( __freebsd__ )
	return (void *)(cxt->uc_mcontext.mc_eip);
#else
	#error uC++ : internal error, unsupported architecture
#endif // OS

#elif defined( __x86_64__ )
#if defined( __linux__ )
	return (void *)(cxt->uc_mcontext.gregs[REG_RIP]);
#elif defined( __freebsd__ )
	return (void *)(cxt->uc_mcontext.mc_rip);
#else
	#error uC++ : internal error, unsupported architecture
#endif // OS

#elif defined( __ia64__ )
	return (void *)(cxt->uc_mcontext.sc_ip);

#elif defined( __sparc__ )
	return (void *)(cxt->uc_mcontext.gregs[REG_PC]);
#else
	#error uC++ : internal error, unsupported architecture
#endif // architecture
    } // uSigHandlerModule::signalContextPC


    void uSigHandlerModule::sigTermHandler( __U_SIGTYPE__ ) {
	// This routine handles a SIGHUP, SIGINT, or a SIGTERM signal.  The signal is delivered to the root process as
	// the result of some action on the part of the user attempting to terminate the application.  It must be caught
	// here so that all processes in the application may be terminated.

#ifdef __U_DEBUG_H__
	char buffer[256];
	uDebugPrtBuf( buffer, "sigTermHandler, cluster:%.128s (%p), processor:%p\n",
		      uThisCluster().getName(), &uThisCluster(), &uThisProcessor() );
#endif // __U_DEBUG_H__

#ifdef __U_STATISTICS__
	if ( Statistics::prtSigterm ) Statistics::print();
	if ( Statistics::prtHeapterm ) uHeapManager::print();
#endif // __U_STATISTICS__

      if ( uKernelModule::globalAbort ) return;		// close down in progress, ignore signal

	uAbort( "Application interrupted by a termination signal." );
    } // uSigHandlerModule::sigTermHandler


    static inline
#if defined( __linux__ )

#if ! defined( __ia64__ )
	greg_t
#else
	long int
#endif // __ia64__

#elif defined( __freebsd__ )
	__register_t
#elif defined( __solaris__ )
	greg_t
#else
	#error uC++ : internal error, unsupported architecture
#endif
    getSP( __U_SIGCXT__ cxt ) {
#if defined( __linux__ )

#if ! defined( __ia64__ )
	return cxt->uc_mcontext.gregs[
#if __U_WORDSIZE__ == 32
	    REG_ESP
#else
	    REG_RSP
#endif // __U_WORDSIZE__ == 32
	    ];
#else
	return cxt->uc_mcontext.sc_gr[12];
#endif // __ia64__

#elif defined( __freebsd__ )
	return cxt->uc_mcontext.
#if __U_WORDSIZE__ == 32
	    mc_esp;
#else
	    mc_rsp;
#endif // __U_WORDSIZE__ == 32

#elif defined( __solaris__ )
	return cxt->uc_mcontext.gregs[REG_SP];
#else
	#error uC++ : internal error, unsupported architecture
#endif // operating systems
    } // uSigHandlerModule::getSP


    void uSigHandlerModule::sigAlrmHandler( __U_SIGPARMS__ ) {
	// This routine handles a SIGALRM/SIGUSR1 signal.  This signal is delivered as the result of time slicing being
	// enabled, a processor is being woken up after being idle for some time, or some intervention being delivered
	// to a thread.  This handler attempts to yield the currently executing thread so that another thread may be
	// scheduled by this processor.

#ifdef __U_DEBUG_H__
	char buffer[512];
	uDebugPrtBuf( buffer, "sigAlrmHandler, signal:%d, errno:%d, cluster:%p (%s), processor:%p, task:%p (%s), stack:0x%lx, address:%p, RFpending:%d, RFinprogress:%d, disableInt:%d, disableIntCnt:%d, disableIntSpin:%d, disableIntSpinCnt:%d\n",
			sig, errno, &uThisCluster(), uThisCluster().getName(), &uThisProcessor(), &uThisTask(), uThisTask().getName(), getSP( cxt ), signalContextPC( cxt ), THREAD_GETMEM( RFpending ), THREAD_GETMEM( RFinprogress ), THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ), THREAD_GETMEM( disableIntSpin ), THREAD_GETMEM( disableIntSpinCnt ) );
#endif // __U_DEBUG_H__

#ifdef __U_STATISTICS__
	if ( sig == SIGUSR1 ) {
	    uFetchAdd( UPP::Statistics::signal_usr1, 1 );
	} else if ( sig == SIGALRM ) {
	    uFetchAdd( UPP::Statistics::signal_alarm, 1 );
	} else {
	    uAbort( "UNKNOWN ALARM SIGNAL\n" );
	} // if
#endif // __U_STATISTICS__

#if defined( __ia64__ ) && defined( __U_MULTI__ )
	// The following check must be executed first on the ia64. It clears p7 to indicate a signal occured during a
	// THREAD_GETMEM / THREAD_SETMEM.
      if ( *(unsigned long*)cxt->uc_mcontext.sc_gr[13] & 1 ) {
	    cxt->uc_mcontext.sc_pr &= ~(1 << 7);
	    return;
	} // if
#endif // __ia64__ && __U_MULTI__

      if ( uKernelModule::globalAbort ) return;		// close down in progress, ignore signal

	int terrno = errno;				// preserve errno at point of interrupt

      if ( THREAD_GETMEM( RFinprogress ) ||		// roll forward in progress ?
	   THREAD_GETMEM( disableInt ) ||		// inside kernel ?
	   THREAD_GETMEM( disableIntSpin ) ) {		// spinlock acquired ?
#ifdef __U_DEBUG_H__
	    uDebugPrtBuf( buffer, "sigAlrmHandler1, signal:%d\n", sig );
	    errno = terrno;				// reset errno and continue
#endif // __U_DEBUG_H__
	    THREAD_SETMEM( RFpending, true );		// indicate roll forward is required
	    return;
	} // if

	// Unsafe to perform these checks if in kernel or performing roll forward, because the thread specific variables
	// used by uThis* routines are changing.
#if defined( __U_DEBUG__ ) && defined( __U_MULTI__ )
	if ( sig == SIGALRM ) {				// only handle SIGALRM on system cluster
	    assert( &uThisProcessor() == uKernelModule::systemProcessor );
	    assert( &uThisCluster() == uKernelModule::systemCluster );
	} // if
#endif // __U_DEBUG__ && __U_MULTI__
#if defined( __U_DEBUG__ )
	uThisCoroutine().verify();			// good place to check for stack overflow
#endif // __U_DEBUG__

#if defined( __U_MULTI__ )
	if ( &uThisProcessor() != uKernelModule::systemProcessor ) {
	    THREAD_SETMEM( RFinprogress, true );	// starting roll forward
	} // if
#endif // __U_MULTI__

	// Clear blocked SIGALRM/SIGUSR1 so more can arrive.
	if ( sizeof( sigset_t ) != sizeof( cxt->uc_sigmask ) ) { // should disappear due to constant folding
	    // uc_sigmask is incorrect size
	    sigset_t new_mask;
	    sigemptyset( &new_mask );
	    if ( &uThisProcessor() == uKernelModule::systemProcessor ) {
		sigaddset( &new_mask, SIGALRM );
	    } // if
	    sigaddset( &new_mask, SIGUSR1 );
	    if ( sigprocmask( SIG_UNBLOCK, &new_mask, NULL ) == -1 ) {
		uAbort( "internal error, sigprocmask" );
	    } // if
	} else {
	    if ( sigprocmask( SIG_SETMASK, (sigset_t *)&(cxt->uc_sigmask), NULL ) == -1 ) {
		uAbort( "internal error, sigprocmask" );
	    } // if
	} // if

#ifdef __U_DEBUG_H__
	uDebugPrtBuf( buffer, "sigAlrmHandler2, signal:%d\n", sig );
#endif // __U_DEBUG_H__

#if __U_LOCALDEBUGGER_H__
	// The current PC is stored, so that it can be looked up by the local debugger to check if the task was time
	// sliced at a breakpoint location.
	uThisTask().debugPCandSRR = signalContextPC( cxt );
#endif // __U_LOCALDEBUGGER_H__

	uKernelModule::rollForward();

#if __U_LOCALDEBUGGER_H__
	// Reset this field before task is started, to denote that this task is not blocked.
	uThisTask().debugPCandSRR = NULL;
#endif // __U_LOCALDEBUGGER_H__

	// Block all signals from arriving so values can be safely reset.
	if ( sigprocmask( SIG_BLOCK, &block_mask, NULL ) == -1 ) {
	    uAbort( "internal error, sigprocmask" );
	} // if

#if defined( __U_MULTI__ ) && defined( __U_SWAPCONTEXT__ )
#   if defined( __linux__ ) && defined( __ia64__ )
	((ucontext_t *)cxt)->uc_mcontext.sc_gr[13] = THREAD_GETMEM( threadPointer );
#   else
	#error uC++ : internal error, unsupported architecture
#   endif
#endif // __U_MULTI__ && __U_SWAPCONTEXT__

#ifdef __U_DEBUG_H__
	uDebugPrtBuf( buffer, "sigAlrmHandler3, signal:%d, errno:%d, cluster:%p (%s), processor:%p, task:%p (%s), stack:0x%lx, address:%p, RFpending:%d, RFinprogress:%d, disableInt:%d, disableIntCnt:%d, disableIntSpin:%d, disableIntSpinCnt:%d\n",
			sig, errno, &uThisCluster(), uThisCluster().getName(), &uThisProcessor(), &uThisTask(), uThisTask().getName(), getSP( cxt ), signalContextPC( cxt ), THREAD_GETMEM( RFpending ), THREAD_GETMEM( RFinprogress ), THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ), THREAD_GETMEM( disableIntSpin ), THREAD_GETMEM( disableIntSpinCnt ) );
#endif // __U_DEBUG_H__
	errno = terrno;					// reset errno and continue
    } // uSigHandlerModule::sigAlrmHandler


    void uSigHandlerModule::sigSegvBusHandler( __U_SIGPARMS__ ) {
      if ( uKernelModule::globalAbort ) _exit( EXIT_FAILURE );	// close down in progress and failed, shutdown immediately!
	uAbort( "Attempt to address location %p.\n"
		"Possible cause is reading outside the address space or writing to a protected area within the address space with an invalid pointer or subscript.",
		sfp->si_addr );
    } // uSigHandlerModule::sigSegvBusHandler


    void uSigHandlerModule::sigIllHandler( __U_SIGPARMS__ ) {
      if ( uKernelModule::globalAbort ) _exit( EXIT_FAILURE );	// close down in progress and failed, shutdown immediately!
	uAbort( "Attempt to execute code at location %p.\n"
		"Possible cause is stack corruption.",
		sfp->si_addr );
    } // uSigHandlerModule::sigIllHandler


    void uSigHandlerModule::sigFpeHandler( __U_SIGPARMS__ ) {
      if ( uKernelModule::globalAbort ) _exit( EXIT_FAILURE );	// close down in progress and failed, shutdown immediately!
	const char *msg;
	switch ( sfp->si_code ) {
	  case FPE_INTDIV:
	  case FPE_FLTDIV: msg = "divide by zero"; break;
	  case FPE_FLTOVF: msg = "overflow"; break;
	  case FPE_FLTUND: msg = "underflow"; break;
	  case FPE_FLTRES: msg = "inexact result"; break;
	  case FPE_FLTINV: msg = "invalid operation"; break;
	  default: msg = "unknown";
	} // switch
	uAbort( "Floating point error.\n"
		"Cause is %s.", msg );
    } // uSigHandlerModule::sigFpeHandler


    uSigHandlerModule::uSigHandlerModule() {
#ifdef __U_DEBUG__
	// When a stack overflow encounters the sentinel page (debug only), there is no stack to deliver the signal.
	// Hence errors that result in terminate are delivered on a separate stack.
	static char stack[SIGSTKSZ];
	static stack_t ss;
	ss.ss_sp = stack;
	ss.ss_size = SIGSTKSZ;
	ss.ss_flags = 0;
	if ( sigaltstack( &ss, NULL ) == -1 ) {
	    uAbort( "uSigHandlerModule::uSigHandlerModule : internal error, sigaltstack error(%d) %s.", errno, strerror( errno ) );
	} // if
#define ONSTACK SA_ONSTACK
#else
#define ONSTACK 0
#endif // __U_DEBUG__

	// Associate handlers with the set of signals that this application is interested in.  These handlers are
	// inherited by all unix processes that are subsequently created so they need not be installed again.

	signal( SIGHUP,  sigTermHandler, SA_SIGINFO | ONSTACK );
	signal( SIGINT,  sigTermHandler, SA_SIGINFO | ONSTACK );
	signal( SIGTERM, sigTermHandler, SA_SIGINFO | ONSTACK );
	signal( SIGSEGV, sigSegvBusHandler, SA_SIGINFO | ONSTACK );
	signal( SIGBUS,  sigSegvBusHandler, SA_SIGINFO | ONSTACK );
	signal( SIGILL,  sigIllHandler, SA_SIGINFO | ONSTACK );
	signal( SIGFPE,  sigFpeHandler, SA_SIGINFO | ONSTACK );

	// Do NOT specify SA_RESTART for SIGALRM because "select" does not wake up when sent a SIGALRM from another UNIX
	// process, which means non-blocking I/O does not work correctly in multiprocessor mode.

	signal( SIGALRM, sigAlrmHandler, SA_SIGINFO );
	signal( SIGUSR1, sigAlrmHandler, SA_SIGINFO );

	sigfillset( &block_mask );			// turn all bits on
    } // uSigHandlerModule::uSigHandlerModule
} // UPP


// Local Variables: //
// compile-command: "make install" //
// End: //
