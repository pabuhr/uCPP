//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Philipp E. Lim 1996
// 
// Sleep.cc -- 
// 
// Author           : Philipp E. Lim
// Created On       : Wed Jan 10 17:02:39 1996
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Dec 19 23:03:18 2016
// Update Count     : 23
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

uClock Clock;
volatile int x = 0, y = 1;

_Task fred {
    void main() {
	uTime start, end;
	for ( ;; ) {
	  if ( x == 20 ) break;
	    if ( x < y ) x += 1;
	    start = Clock.getTime();
	    _Timeout( uDuration( 1 ) );
	    end = Clock.getTime();
	    osacquire( cout ) << "fred slept for " << end - start << " seconds" << endl;
	} // for
	osacquire( cout ) << "fred finished" << endl;
    } // fred::main
}; // fred

_Task mary {
    void main() {
	uTime start, end;
	for ( ;; ) {
	  if ( y == 20 ) break;
	    if ( y == x ) y += 1;
	    start = Clock.getTime();
	    _Timeout( uDuration( 2 ) );
	    end = Clock.getTime();
	    osacquire( cout ) << "mary slept for " << end - start << " seconds" << endl;
	} // for
	osacquire( cout ) << "mary finished" << endl;
    } // mary::main
}; // mary

int main() {
    fred f;
    mary m;
} // main

// Local Variables: //
// compile-command: "u++ Sleep.cc" //
// End: //
