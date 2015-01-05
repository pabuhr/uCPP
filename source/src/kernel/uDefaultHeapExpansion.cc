//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// uDefaultHeapExpansion.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Feb 11 21:18:54 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon May 19 22:40:05 2008
// Update Count     : 18
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


// Must be a separate translation unit so that an application can redefine this routine and the loader does not link
// this routine from the uC++ standard library.


unsigned int uDefaultHeapExpansion() {
    return __U_DEFAULT_HEAP_EXPANSION__;
} // uDefaultHeapExpansion


// Local Variables: //
// compile-command: "make install" //
// End: //
