//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Richard C. Bilson 2007
// 
// Atomic.cc -- 
// 
// Author           : Richard C. Bilson
// Created On       : Mon Sep 10 16:47:22 2007
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Sep 26 01:36:35 2012
// Update Count     : 15
// 

#include <iostream>
using namespace std;

#define NPROCS 8
#define NTASKS 8
#define NITER 1000000

volatile int locn1 = 0, locn2 = 0;

_Task IncTester {
    void main() {
	int cur;
	for ( int i = 0; i < NITER; i += 1 ) {
	    uFetchAdd( locn1, 1 );
	    uFetchAdd( locn1, -1 );

	    do {
		cur = locn2;
	    } while( ! uCompareAssign( locn2, cur, cur + 1 ) );
	    do {
		cur = locn2;
	    } while( ! uCompareAssign( locn2, cur, cur - 1 ) );
	} // for
    } // IncTester::main
}; // IncTester

void uMain::main() {
    uProcessor p[ NPROCS - 1 ] __attribute__(( unused ));
    {
	IncTester testers[ NTASKS ] __attribute__(( unused ));
    }
    if ( locn1 == 0 && locn2 == 0 ) {
	cout << "successful completion" << endl;
    } else {
	cout << "error: expected values 0, 0 but got values " << locn1 << ", " << locn2 << endl;
    } // if
} // uMain::main
