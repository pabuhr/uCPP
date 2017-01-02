//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2010
// 
// UncaughtException.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sun Jul 18 11:14:42 2010
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Dec 19 23:04:37 2016
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

#include <iostream>
using namespace std;

_Task T1 {
    void main() {
	for( ;; ) {
	    _Accept( ~T1 ) break;
	    try {
		throw 1;
	    } catch( int ) {
	    }
	}
    }
};

_Task T2 {
    void main() {
	for( ;; ) {
	    _Accept( ~T2 ) break;
	    assert( ! std::uncaught_exception() );
	    yield();
	}
    }
};

void uMain::main() {
    T1 t1;
    T2 t2;
    uThisTask().uSleep( uDuration( 5 ) );
    cout << "Successful completion" << endl;
}
