//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uBootTask.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Apr 28 11:54:04 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Apr 27 18:27:43 2008
// Update Count     : 29
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


#ifndef __U_BOOTTASK_H__
#define __U_BOOTTASK_H__


namespace UPP {
    _Task uBootTask {
	friend class uKernelBoot;			// access: new

	void main();

	void *operator new( size_t, void *storage );
	void *operator new( size_t size );
      public:
	uBootTask();
	~uBootTask();
    }; // uBootTask
} // UPP


#endif // __U_BOOTTASK_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
