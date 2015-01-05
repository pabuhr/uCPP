//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// Fib.cc -- Produce the fibonacci numbers in sequence on each call.
//
//  No explicit states, communication with argument-parameter mechanism between suspend and resume
//
//  Demonstrate multiple instances of the same coroutine.
// 
// Author           : Peter A. Buhr
// Created On       : Thu Aug  2 11:55:37 1990
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 18 11:05:18 2010
// Update Count     : 40
// 

#include <iostream>
using std::cout;
using std::endl;

_Coroutine fibonacci {
    int fn;

    void main() {
	int fn1, fn2;

	fn = 1;						// special case f0
	fn1 = fn;
	suspend();
	fn = 1;						// special case f1
	fn2 = fn1;
	fn1 = fn;
	suspend();
	for ( ;; ) {					// general case fn
	    fn = fn1 + fn2;
	    suspend();
	    fn2 = fn1;
	    fn1 = fn;
	} // for
    } // fibonacci::main
  public:
    int next() {
	resume();
	return fn;
    }; // next
}; // fibonacci

void uMain::main() {
    const int NoOfFibs = 10;
    fibonacci f1, f2;					// create two fibonacci generators
    int i;

    cout << "Fibonacci Numbers" << endl;
    for ( i = 1; i <= NoOfFibs; i += 1 ) {
	cout << f1.next() << " " << f2.next() << endl;
    } // for
    cout << "successful completion" << endl;
} // uMain::main

// Local Variables: //
// compile-command: "u++ Fib.cc" //
// End: //
