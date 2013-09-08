//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Richard C. Bilson 2006
// 
// uDefaultProcessors-OpenMP.cc -- 
// 
// Author           : Richard C. Bilson
// Created On       : Tue Aug  8 16:53:43 2006
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Jul 12 01:31:11 2012
// Update Count     : 5
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


#include <uDefault.h>
#include <unistd.h>                                     // sysconf
#include <stdlib.h>                                     // getenv, atoi


// Must be a separate translation unit so that an application can redefine this routine and the loader does not link
// this routine from the uC++ standard library.


// OpenMP creates one kernel thread for each physical processor by default, which can be overridden by specifying the
// OMP_NUM_THREADS environment variable.

unsigned int uDefaultProcessors() {
#if defined( __linux__ ) || defined( __freebsd__ )
    unsigned int nprocs = 0;
    char *value = getenv( "OMP_NUM_THREADS" );
    if ( value != NULL ) {
	nprocs = atoi( value );
    } else {
	nprocs = sysconf( _SC_NPROCESSORS_ONLN );
    } // if
    return __U_DEFAULT_PROCESSORS__ > nprocs ? __U_DEFAULT_PROCESSORS__ : nprocs;
#else
    return __U_DEFAULT_PROCESSORS__;
#endif // __linux__
} // uDefaultProcessors


// Local Variables: //
// compile-command: "make install" //
// End: //
