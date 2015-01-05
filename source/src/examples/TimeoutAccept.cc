//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Ashif S. Harji 1997
// 
// TimeoutAccept.cc -- 
// 
// Author           : Ashif S. Harji
// Created On       : Thu Dec 11 10:17:16 1997
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Sep 11 16:18:12 2008
// Update Count     : 98
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Task T1;						// forward declarations
_Task T2;

const unsigned int NoOfTimes = 20;

_Task T1 {
    T2 *t2;

    void main();
  public:
    void mem1() {}
    void mem2() {}

    void partner( T2 &t ) {
	t2 = &t;
    } // partner
}; // T1

_Task T2 {
    T1 &t1;

    void main();
  public:
    T2( T1 &t1 ) : t1( t1 ) {}
    void cont() {}
}; // T2

void T1::main(){
    uTime starttime, endtime;

    _Accept( partner );				// close reference cycle

    // Test if timing out works.

    starttime = uThisProcessor().getClock().getTime();
    
    _Accept( mem1 ){
	osacquire( cout ) << this << " mem1 accepted" << endl;
    } or _Accept( mem2 ){
	osacquire( cout ) << this << " mem2 accepted" << endl;
    } or _Timeout( uDuration( 1 ) ) {
	osacquire( cout ) << this << " timeout accepted" << endl;
    } // _Accept
    endtime = uThisProcessor().getClock().getTime();
    osacquire( cout ) << this << " ending at " << endtime - starttime << endl;

    t2->cont();

    // Test calls which occur increasingly close to timeout value.

    for ( unsigned int i = 0; i < NoOfTimes + 3; i += 1 ) {
	starttime = uThisProcessor().getClock().getTime();
	
	_Accept( mem1 ) {
	    osacquire( cout ) << this << " mem1 accepted" << endl;
	} or _Accept( mem2 ){
	    osacquire( cout ) << this << " mem2 accepted" << endl;
	} or _Timeout( uDuration( 1 ) ) {
	    osacquire( cout ) << this << " timeout accepted" << endl;

	    _Accept( mem1 ) {				// receive the call after the timeout
	    } or _Accept( mem2 );
	} // _Accept

	endtime = uThisProcessor().getClock().getTime();
	if ( i < 4 ) {
	    osacquire( cout ) << this << " ending at " << endtime - starttime << endl;
	} // if

	t2->cont();
    } // for
} // t1::main

void T2::main(){
    // Test if timing out works.

    _Accept( cont );

    // Test calls which occur increasingly close to timeout value.

    _Timeout( uDuration( 0, 100000000 ) );
    t1.mem1();
    _Accept( cont );

    _Timeout( uDuration( 0, 500000000 ) );
    t1.mem2();
    _Accept( cont );

    _Timeout( uDuration( 0, 900000000 ) );
    t1.mem1();
    _Accept( cont );

    for ( unsigned int i = 0; i < NoOfTimes; i += 1 ) {
	_Timeout( uDuration( 0, 999995000 ) );
	t1.mem2();
	_Accept( cont );
    } // for
} // T2::main

void uMain::main(){
    uProcessor processor[1] __attribute__(( unused ));	// more than one processor
    T1 r1;
    T2 s1( r1 );
    T1 r2;
    T2 s2( r2 );

    r1.partner( s1 );
    r2.partner( s2 );
} // uMain::main
