//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uDebug.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sat Dec 18 13:04:26 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Sep 22 21:55:25 2019
// Update Count     : 140
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
#include <uDebug.h>

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdarg>
#include <unistd.h>					// write


using namespace UPP;


// This debug print uses its own buffer or a supplied buffer to build the output string to ensure that there are no
// calls to malloc at the file buffer level. Therefore, it is safe to call this routine from inside the memory
// management routines because there will be no recursive calls to allocate memory.

enum { BufferSize = 4096 };
static char buffer[BufferSize];

// SKULLDUGGERY: The debug spin lock has to be available at the same time as the heap routines to allow debugging
// them. Since the initial value of a spin lock are zero and static storage is initialized to zero, this works out.

static char uDebugLockStorage[sizeof(uSpinLock)] __attribute__(( aligned (16) ));
#define uDebugLock ((uSpinLock *)&uDebugLockStorage)


extern "C" void uDebugWrite( int fd, const char *buffer, int len ) {
    for ( int count = 0, retcode; count < len; count += retcode ) { // ensure all data is written
	buffer += count;
	for ( ;; ) {
	    retcode = ::write( fd, buffer, len - count );
	  if ( retcode != -1 || errno != EINTR ) break; // not a timer interrupt ?
	} // for
	if ( retcode == -1 ) _exit( EXIT_FAILURE );
    } // for
} // uDebugWrite


extern "C" void uDebugAcquire() {
    uDebugLock->acquire();
    int len = sprintf( ::buffer, "(%ld) ", (long int)
#if defined( __U_MULTI__ )
	     RealRtn::pthread_self()
#else
	     getpid()
#endif // __U_MULTI__
	);
    uDebugWrite( STDERR_FILENO, ::buffer, len );
} // uDebugAcquire


extern "C" void uDebugRelease() {
    uDebugLock->release();
} // uDebugRelease


extern "C" void uDebugPrt( const char fmt[], ... ) {
    va_list args;

    va_start( args, fmt );
    uDebugLock->acquire();
    int len = sprintf( ::buffer, "(%ld) ", (long int)
#if defined( __U_MULTI__ )
	     // null if called early in boot sequence
	     (RealRtn::pthread_self == nullptr ? 0 : RealRtn::pthread_self())
#else
	     getpid()
#endif // __U_MULTI__
	);
    len += vsnprintf( ::buffer + len, BufferSize, fmt, args );	// and after that the message
    uDebugWrite( STDERR_FILENO, ::buffer, len );
    uDebugRelease();
    va_end( args );
} // uDebugPrt


// No lock to allow printing after explicitly acquiring the lock.

extern "C" void uDebugPrt2( const char fmt[], ... ) {
    va_list args;

    va_start( args, fmt );
    int len = vsnprintf( ::buffer, BufferSize, fmt, args );
    uDebugWrite( STDERR_FILENO, ::buffer, len );
    va_end( args );
} // uDebugPrt2


// No lock to allow printing in potential deadlock situations.

extern "C" void uDebugPrtBuf( char buffer[], const char fmt[], ... ) {
    va_list args;

    va_start( args, fmt );
    int len = sprintf( buffer, "(%ld) ", (long int)
#if defined( __U_MULTI__ )
	     RealRtn::pthread_self()
#else
	     getpid()
#endif // __U_MULTI__
	);
    len += vsnprintf( buffer + len, BufferSize, fmt, args ); // and after that the message
    uDebugWrite( STDERR_FILENO, buffer, len );
    va_end( args );
} // uDebugPrt


// Local Variables: //
// compile-command: "make install" //
// End: //
