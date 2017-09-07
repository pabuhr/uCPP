//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uSignal.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sun Dec 19 16:32:13 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Mar 13 07:59:19 2017
// Update Count     : 850
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

#if defined( __linux__ )
#include <execinfo.h>					// backtrace, backtrace_symbols
#include <cxxabi.h>					// __cxa_demangle
#endif // __linux__

namespace UPP {
    sigset_t uSigHandlerModule::block_mask;

#if defined( __linux__ )
    static void uBacktrace( int start ) {		// skip first N stack frames
	enum { Frames = 50 };
	void * array[Frames];
	int size = ::backtrace( array, Frames );
	char ** messages = ::backtrace_symbols( array, size ); // does not demangle names
	char helpText[256];
	int len;

	*index( messages[0], '(' ) = '\0';		// find executable name
	len = snprintf( helpText, 256, "Stack trace for: %s\n", messages[0] );
	uDebugWrite( STDERR_FILENO, helpText, len );

	// skip last stack frame after uMain
	for ( int i = start; i < size - 1 && messages != nullptr; i += 1 ) {
	    char * mangled_name = nullptr, * offset_begin = nullptr, * offset_end = nullptr;
	    for ( char *p = messages[i]; *p; ++p ) {	// find parantheses and +offset
		if ( *p == '(' ) {
		    mangled_name = p;
		} else if ( *p == '+' ) {
		    offset_begin = p;
		} else if ( *p == ')' ) {
		    offset_end = p;
		    break;
		} // if
	    } // for

	    // if line contains symbol, attempt to demangle
	    int frameNo = i - start;
	    if ( mangled_name && offset_begin && offset_end && mangled_name < offset_begin ) {
		*mangled_name++ = '\0';			// delimit strings
		*offset_begin++ = '\0';
		*offset_end++ = '\0';

		int status;
		char * real_name = __cxxabiv1::__cxa_demangle( mangled_name, 0, 0, &status );
		// bug in __cxa_demangle for single-character lower-case non-mangled names
		if ( status == 0 ) {			// demangling successful ?
		    len = snprintf( helpText, 256, "(%d) %s %s+%s%s\n",
				    frameNo, messages[i], real_name, offset_begin, offset_end );
		} else {				// otherwise, output mangled name
		    len = snprintf( helpText, 256, "(%d) %s %s(/*unknown*/)+%s%s\n",
				    frameNo, messages[i], mangled_name, offset_begin, offset_end );
		} // if

		free( real_name );
	    } else {					// otherwise, print the whole line
		len = snprintf( helpText, 256, "(%d) %s\n", frameNo, messages[i] );
	    } // if
	    uDebugWrite( STDERR_FILENO, helpText, len );
	} // for

	free( messages );
    } // uBacktrace
#endif // __linux__


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

	if ( sigaction( sig, &act, nullptr ) == -1 ) {
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

	uDEBUGPRT(
	    char buffer[256];
	    uDebugPrtBuf( buffer, "sigTermHandler, cluster:%.128s (%p), processor:%p\n",
			  uThisCluster().getName(), &uThisCluster(), &uThisProcessor() );
	)

#ifdef __U_STATISTICS__
	if ( Statistics::prtSigterm ) Statistics::print();
	if ( Statistics::prtHeapterm ) uHeapManager::print();
#endif // __U_STATISTICS__

      if ( uKernelModule::globalAbort ) return;		// close down in progress, ignore signal

	abort( "Application interrupted by a termination signal." );
    } // uSigHandlerModule::sigTermHandler


#if 0
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
#endif // 0


    void uSigHandlerModule::sigAlrmHandler( __U_SIGPARMS__ ) {
	// This routine handles a SIGALRM/SIGUSR1 signal.  This signal is delivered as the result of time slicing being
	// enabled, a processor is being woken up after being idle for some time, or some intervention being delivered
	// to a thread.  This handler attempts to yield the currently executing thread so that another thread may be
	// scheduled by this processor.

	uDEBUGPRT(
	    char buffer[512];
	    uDebugPrtBuf( buffer, "sigAlrmHandler, signal:%d, errno:%d, cluster:%p (%s), processor:%p, task:%p (%s), stack:0x%lx, address:%p, RFpending:%d, RFinprogress:%d, disableInt:%d, disableIntCnt:%d, disableIntSpin:%d, disableIntSpinCnt:%d\n",
			  sig, errno, &uThisCluster(), uThisCluster().getName(), &uThisProcessor(), &uThisTask(), uThisTask().getName(), getSP( cxt ), signalContextPC( cxt ), THREAD_GETMEM( RFpending ), THREAD_GETMEM( RFinprogress ), THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ), THREAD_GETMEM( disableIntSpin ), THREAD_GETMEM( disableIntSpinCnt ) );
	)

#ifdef __U_STATISTICS__
	if ( sig == SIGUSR1 ) {
	    uFetchAdd( UPP::Statistics::signal_usr1, 1 );
	} else if ( sig == SIGALRM ) {
	    uFetchAdd( UPP::Statistics::signal_alarm, 1 );
	} else {
	    abort( "UNKNOWN ALARM SIGNAL\n" );
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
	    uDEBUGPRT( uDebugPrtBuf( buffer, "sigAlrmHandler1, signal:%d\n", sig ); )
	    errno = terrno;				// reset errno and continue )

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
	    if ( sigprocmask( SIG_UNBLOCK, &new_mask, nullptr ) == -1 ) {
		abort( "internal error, sigprocmask" );
	    } // if
	} else {
	    if ( sigprocmask( SIG_SETMASK, (sigset_t *)&(cxt->uc_sigmask), nullptr ) == -1 ) {
		abort( "internal error, sigprocmask" );
	    } // if
	} // if

	uDEBUGPRT( uDebugPrtBuf( buffer, "sigAlrmHandler2, signal:%d\n", sig ); )


#if __U_LOCALDEBUGGER_H__
	// The current PC is stored, so that it can be looked up by the local debugger to check if the task was time
	// sliced at a breakpoint location.
	uThisTask().debugPCandSRR = signalContextPC( cxt );
#endif // __U_LOCALDEBUGGER_H__

	uKernelModule::rollForward();

#if __U_LOCALDEBUGGER_H__
	// Reset this field before task is started, to denote that this task is not blocked.
	uThisTask().debugPCandSRR = nullptr;
#endif // __U_LOCALDEBUGGER_H__

	// Block all signals from arriving so values can be safely reset.
	if ( sigprocmask( SIG_BLOCK, &block_mask, nullptr ) == -1 ) {
	    abort( "internal error, sigprocmask" );
	} // if

#if defined( __U_MULTI__ ) && defined( __U_SWAPCONTEXT__ )
#   if defined( __linux__ ) && defined( __ia64__ )
	((ucontext_t *)cxt)->uc_mcontext.sc_gr[13] = THREAD_GETMEM( threadPointer );
#   else
	#error uC++ : internal error, unsupported architecture
#   endif
#endif // __U_MULTI__ && __U_SWAPCONTEXT__

	uDEBUGPRT(
	    uDebugPrtBuf( buffer, "sigAlrmHandler3, signal:%d, errno:%d, cluster:%p (%s), processor:%p, task:%p (%s), stack:0x%lx, address:%p, RFpending:%d, RFinprogress:%d, disableInt:%d, disableIntCnt:%d, disableIntSpin:%d, disableIntSpinCnt:%d\n",
			  sig, errno, &uThisCluster(), uThisCluster().getName(), &uThisProcessor(), &uThisTask(), uThisTask().getName(), getSP( cxt ), signalContextPC( cxt ), THREAD_GETMEM( RFpending ), THREAD_GETMEM( RFinprogress ), THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ), THREAD_GETMEM( disableIntSpin ), THREAD_GETMEM( disableIntSpinCnt ) );
	)

	errno = terrno;					// reset errno and continue
    } // uSigHandlerModule::sigAlrmHandler


    void uSigHandlerModule::sigSegvBusHandler( __U_SIGPARMS__ ) {
      if ( uKernelModule::globalAbort ) _exit( EXIT_FAILURE ); // close down in progress and failed, shutdown immediately!
#if defined( __linux__ )
	uBacktrace( 3 );				// skip first N stack frames
#endif // __linux__
	abort( "Attempt to address location %p.\n"
		"Possible cause is reading outside the address space or writing to a protected area within the address space with an invalid pointer or subscript.",
		sfp->si_addr );
    } // uSigHandlerModule::sigSegvBusHandler


    void uSigHandlerModule::sigIllHandler( __U_SIGPARMS__ ) {
      if ( uKernelModule::globalAbort ) _exit( EXIT_FAILURE ); // close down in progress and failed, shutdown immediately!
	abort( "Attempt to execute code at location %p.\n"
		"Possible cause is stack corruption.",
		sfp->si_addr );
    } // uSigHandlerModule::sigIllHandler


    void uSigHandlerModule::sigFpeHandler( __U_SIGPARMS__ ) {
      if ( uKernelModule::globalAbort ) _exit( EXIT_FAILURE ); // close down in progress and failed, shutdown immediately!
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
	abort( "Floating point error.\n"
		"Cause is %s.", msg );
    } // uSigHandlerModule::sigFpeHandler


    uSigHandlerModule::uSigHandlerModule() {
	// As a precaution (and necessity), errors that result in termination are delivered on a separate stack because
	// task stacks might be very small (4K) and the signal delivery corrupts memory to the point that a clean
	// shutdown is impossible. Also, when a stack overflow encounters the non-accessible sentinel page (debug only)
	// and generates a segment fault, the signal cannot be delivered on the sentinel page.
	static char stack[SIGSTKSZ] __attribute__(( aligned (16) ));
	static stack_t ss;
	uDEBUGPRT( uDebugPrt( "uSigHandlerModule, stack:%p, size:%d, %p\n", stack, SIGSTKSZ, stack + SIGSTKSZ ); )

	ss.ss_sp = stack;
	ss.ss_size = SIGSTKSZ;
	ss.ss_flags = 0;
	if ( sigaltstack( &ss, nullptr ) == -1 ) {
	    abort( "uSigHandlerModule::uSigHandlerModule : internal error, sigaltstack error(%d) %s.", errno, strerror( errno ) );
	} // if

	// Associate handlers with the set of signals that this application is interested in.  These handlers are
	// inherited by all unix processes that are subsequently created so they are not installed again.

	signal( SIGHUP,  sigTermHandler, SA_SIGINFO | SA_ONSTACK );
	signal( SIGINT,  sigTermHandler, SA_SIGINFO | SA_ONSTACK );
	signal( SIGTERM, sigTermHandler, SA_SIGINFO | SA_ONSTACK );
	signal( SIGSEGV, sigSegvBusHandler, SA_SIGINFO | SA_ONSTACK );
	signal( SIGBUS,  sigSegvBusHandler, SA_SIGINFO | SA_ONSTACK );
	signal( SIGILL,  sigIllHandler, SA_SIGINFO | SA_ONSTACK );
	signal( SIGFPE,  sigFpeHandler, SA_SIGINFO | SA_ONSTACK );

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
