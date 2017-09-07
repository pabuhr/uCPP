//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// abortExit.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Oct 26 11:54:31 1990
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Apr 29 10:24:16 2017
// Update Count     : 564
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

#include <uDebug.h>					// access: uDebugWrite
#undef __U_DEBUG_H__					// turn off debug prints

#include <cstdio>
#include <cstdarg>
#include <cerrno>
#include <unistd.h>					// _exit

using namespace UPP;


void exit( int retcode ) __THROW {			// interpose
    uKernelModule::retCode = retcode;
    RealRtn::exit( retcode );				// call the real exit
    // CONTROL NEVER REACHES HERE!
} // exit


void abort() __THROW {					// interpose
    abort( nullptr );
    // CONTROL NEVER REACHES HERE!
} // abort


// Only one processor should call abort and succeed.  Once a processor calls abort, all other processors quietly exit
// while the aborting processor cleans up the system and possibly dumps core.

void uAbort( const char *fmt, ... ) {			// deprecated
    va_list args;
    va_start( args, fmt );
    abort( fmt, args );
    va_end( args );
} // uAbort

void abort( const char *fmt, ... ) {
#if defined( __U_MULTI__ )
    // abort cannot be recursively entered by the same or different processors because all signal handlers return when
    // the globalAbort flag is true.
    uKernelModule::globalAbortLock->acquire();
    if ( uKernelModule::globalAbort ) {			// not first task to abort ?
	uKernelModule::globalAbortLock->release();
	sigset_t mask;
	sigemptyset( &mask );
	sigaddset( &mask, SIGALRM );			// block SIGALRM signals
	sigaddset( &mask, SIGUSR1 );			// block SIGUSR1 signals
	sigsuspend( &mask );				// block the processor to prevent further damage during abort
	_exit( EXIT_FAILURE );				// if processor unblocks before it is killed, terminate it
    } else {
	uKernelModule::globalAbort = true;		// first task to abort ?
	uKernelModule::globalAbortLock->release();
    } // if
#endif // __U_MULTI__

    uBaseTask &task = uThisTask();			// optimization

#ifdef __U_PROFILER__
    // profiling

    task.profileInactivate();				// make sure the profiler is not called from this point on
#endif // __U_PROFILER__

#ifdef __U_DEBUG__
    // Turn off uOwnerLock checking.
    uKernelModule::initialized = false;
#endif // __U_DEBUG__

    enum { BufferSize = 1024 };
    static char helpText[BufferSize];
    int len = snprintf( helpText, BufferSize, "uC++ Runtime error (UNIX pid:%ld) ", (long int)getpid() ); // use UNIX pid (versus getPid)
    uDebugWrite( STDERR_FILENO, helpText, len );

    if ( fmt != nullptr ) {
	// Display the relevant shut down information.

	va_list args;
	va_start( args, fmt );
	len = vsnprintf( helpText, BufferSize, fmt, args );
	va_end( args );
	uDebugWrite( STDERR_FILENO, helpText, len );
	helpText[0] = '\n';
	uDebugWrite( STDERR_FILENO, helpText, 1 );
    } // if

    len = snprintf( helpText, BufferSize, "Error occurred while executing task %.256s (%p)", task.getName(), &task );
    uDebugWrite( STDERR_FILENO, helpText, len );
    if ( &task != &uThisCoroutine() ) {
	len = snprintf( helpText, BufferSize, " in coroutine %.256s (%p).\n", uThisCoroutine().getName(), &uThisCoroutine() );
	uDebugWrite( STDERR_FILENO, helpText, len );
    } else {
	helpText[0] = '.'; helpText[1] = '\n';
	uDebugWrite( STDERR_FILENO, helpText, 2 );
    } // if

    // In debugger mode, tell the global debugger to stop the application.

#if __U_LOCALDEBUGGER_H__
    if ( ! THREAD_GETMEM( disableInt ) && ! THREAD_GETMEM( disableIntSpin ) ) {
	if ( uLocalDebugger::uLocalDebuggerActive ) uLocalDebugger::uLocalDebuggerInstance->abortApplication();
    } // if
#endif // __U_LOCALDEBUGGER_H__

    // After having killed off the other processors, dump core if required, otherwise, quietly call "_exit". Cannot call
    // "exit" because of global destructors.

    if ( ! uKernelModule::coreDumped ) {		// child process may have failed and dumped core already
	uKernelModule::coreDumped = true;		// prevent other UNIX processes from dumping core
	RealRtn::abort();				// call the real abort
    } // if

    _exit( EXIT_FAILURE );
    // CONTROL NEVER REACHES HERE!
} // abort


// Local Variables: //
// compile-command: "make install" //
// End: //
