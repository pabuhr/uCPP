//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Philipp E. Lim 1996
// 
// Sleep.cc -- 
// 
// Author           : Philipp E. Lim
// Created On       : Wed Jan 10 17:02:39 1996
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Dec  3 09:41:53 2005
// Update Count     : 22
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

void uMain::main() {
    fred f;
    mary m;
} // uMain::main

// Local Variables: //
// compile-command: "u++ Sleep.cc" //
// End: //
