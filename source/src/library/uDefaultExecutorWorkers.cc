//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr and Thierry Delisle 2016
// 
// uDefaultExecutorWorkers.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Jan  2 20:58:06 2020
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed May 20 16:43:59 2020
// Update Count     : 2
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


#include <uDefaultExecutor.h>


// Must be a separate translation unit so that an application can redefine this routine and the loader does not link
// this routine from the uC++ standard library.


unsigned int uDefaultExecutorWorkers() {
    return __U_DEFAULT_EXECUTOR_WORKERS__;
} // uDefaultExecutorWorkers


// Local Variables: //
// compile-command: "make install" //
// End: //
