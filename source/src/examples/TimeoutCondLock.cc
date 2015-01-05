//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 2007
// 
// TimeoutCondLock.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Tue Jun 26 07:44:49 2007
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Sep 13 11:12:02 2008
// Update Count     : 18
// 

#include <uBarrier.h>
#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

uOwnerLock mutex;
uCondLock waitc;
uBarrier b( 2 );

const unsigned int NoOfTimes = 20;

_Task T1 {
    void main(){
	mutex.acquire();
	waitc.wait( mutex, uDuration( 1 ) );
	osacquire( cout ) << &uThisTask() << " timedout" << endl;

	b.block();

	// Test calls which occur increasingly close to timeout value.

	for ( unsigned int i = 0; i < NoOfTimes + 3; i += 1 ) {
	    if ( waitc.wait( mutex, uDuration( 1 ) ) ) { 
		osacquire( cout ) << &uThisTask() << " signalled" << endl;
	    } else {
		osacquire( cout ) << &uThisTask() << " timedout" << endl;
	    } // if

	    b.block();
	} // for
    } // t1::main
  public:
}; // T1

_Task T2 {
    void main(){
	// Test if timing out works.

	b.block();

	// Test calls which occur increasingly close to timeout value.

	_Timeout( uDuration( 0, 100000000 ) );
	waitc.signal();
	b.block();

	_Timeout( uDuration( 0, 500000000 ) );
	waitc.signal();
	b.block();

	_Timeout( uDuration( 0, 900000000 ) );
	waitc.signal();
	b.block();

	for ( unsigned int i = 0; i < NoOfTimes; i += 1 ) {
	    _Timeout( uDuration( 0, 999950000 ) );
	    waitc.signal();
	    b.block();
	} // for
    } // for
}; // T2::main

void uMain::main(){
    uProcessor processor[1] __attribute__(( unused ));	// more than one processor
    T1 r1;
    T2 r2;
} // uMain::main

// Local Variables: //
// compile-command: "u++ TimeoutCondLock.cc" //
// End: //
