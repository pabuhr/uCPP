//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2019
// 
// uFuture.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Jun  3 18:06:58 2019
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Aug 26 20:42:34 2020
// Update Count     : 8
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
#include <uFuture.h>
unsigned int uExecutor::next = 0;			// demultiplex across worker buffers

// Local Variables: //
// compile-command: "make install" //
// End: //
