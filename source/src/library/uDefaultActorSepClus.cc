//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2017
// 
// uDefaultActorSepClus.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Jul 10 15:02:42 2017
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Jul 10 18:24:22 2017
// Update Count     : 3
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


bool uDefaultActorSepClus() {
    return __U_DEFAULT_ACTOR_SEPCLUS__;			// create processors on separate cluster
} // uDefaultActorSepClus


// Local Variables: //
// compile-command: "make install" //
// End: //
