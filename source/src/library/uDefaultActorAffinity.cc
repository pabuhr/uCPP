//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2017
// 
// uDefaultActorAffinity.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Jul 10 14:43:50 2017
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Jan  2 21:08:52 2019
// Update Count     : 6
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


int uDefaultActorAffinity() {
    return __U_DEFAULT_ACTOR_AFFINITY__;		// affinity and CPU offset (-1 => no affinity, default)
} // uDefaultActorAffinity


// Local Variables: //
// compile-command: "make install" //
// End: //
