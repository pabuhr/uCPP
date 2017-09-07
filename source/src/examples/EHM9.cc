//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2003
// 
// EHM9.cc -- recursive resumption
// 
// Author           : Peter A. Buhr
// Created On       : Sun Dec  7 12:15:30 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jan 22 21:49:40 2017
// Update Count     : 37
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

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Event R1 {};
_Event R2 {};

// Checks marking of handlers is performed correctly during propagation.
// Marking prevents recursive resumption.

int main() {
	try {
		try {
			try {
				osacquire( cout ) << "before raise" << endl;
				_Resume R1();
				osacquire( cout ) << "after raise" << endl;
			} _CatchResume( R2 ) {
				osacquire( cout ) << "enter H3" << endl;
				_Resume R1();
				osacquire( cout ) << "exit  H3" << endl;
			} // try
		} _CatchResume( R1 ) {
			osacquire( cout ) << "enter H2" << endl;
			_Resume R2();
			osacquire( cout ) << "exit  H2" << endl;
		} // try
	} _CatchResume( R2 ) {
		osacquire( cout ) << "enter H1" << endl;
		osacquire( cout ) << "exit  H1" << endl;
	} // try
	osacquire( cout ) << "finished" << endl;
} // main


// Local Variables: //
// tab-width: 4 //
// End: //
