//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 2014
// 
// std_mutex.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri May 13 12:23:47 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri May 13 12:57:32 2016
// Update Count     : 4
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

// This include file uses the uC++ keyword _Mutex as a template parameter name.
// The name is changed only for the include file.

#define _Mutex Mutex_
#include_next <bits/std_mutex.h>
#undef Mutex_

// Local Variables: //
// compile-command: "make install" //
// End: //
