//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// uFloat.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Oct 10 08:30:46 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Sep 12 21:42:10 2009
// Update Count     : 38
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


#ifdef __U_FLOATINGPOINTDATASIZE__
uFloatingPointContext::uFloatingPointContext() : uContext( &uniqueKey ) {
} // uFloatingPointContext::uFloatingPointContext
#endif // __U_FLOATINGPOINTDATASIZE__

#if defined( __ia64__ )
extern "C" void uIA64FPsave( double cxt[] );
extern "C" void uIA64FPrestore( double cxt[] );
#endif // __ia64__

void uFloatingPointContext::save() {
    
#if defined( __i386__ )
    // saved by caller
#elif defined( __x86_64__ )
    // saved by caller
#elif defined( __ia64__ )
#   if ! defined( __U_SWAPCONTEXT__ )
	uIA64FPsave( floatingPointData );
#   endif // ! __U_SWAPCONTEXT__
#elif defined( __sparc__ )
    // saved by caller
#else
    #error uC++ : internal error, unsupported architecture
#endif

} // uFloatingPointContext::save


void uFloatingPointContext::restore() {
    
#if defined( __i386__ )
    // restored by caller
#elif defined( __x86_64__ )
    // restored by caller
#elif defined( __ia64__ )
#if ! defined( __U_SWAPCONTEXT__ )
    uIA64FPrestore( floatingPointData );
#endif // ! __U_SWAPCONTEXT__
#elif defined( __sparc__ )
    // restored by caller
#else
    #error uC++ : internal error, unsupported architecture
#endif

} // uFloatingPointContext::restore


// Local Variables: //
// compile-command: "make install" //
// End: //
