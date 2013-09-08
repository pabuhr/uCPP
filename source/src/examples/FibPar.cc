//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// FibPar.cc -- Produce the fibonacci numbers in sequence on each call.
//
//  No explicit states, communication with argument-parameter mechanism between suspend and resume
//
//  Demonstrate multiple instances of the same coroutine.
//
//  Accessiable by multiple threads
// 
// Author           : Peter A. Buhr
// Created On       : Thu Aug  2 11:55:37 1990
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 18 11:05:27 2010
// Update Count     : 92
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Mutex _Coroutine fibonacci {
    int fn, fn1, fn2;

    void main() {
	fn = 1;						// 1st case
	fn1 = fn;
	suspend();
	fn = 1;						// 2nd case
	fn2 = fn1;
	fn1 = fn;
	suspend();
	for ( ;; ) {					// general case
	    fn = fn1 + fn2;
	    fn2 = fn1;
	    fn1 = fn;
	    suspend();
	} // for
    } // fibonacci::main
  public:
    int next() {
	resume();
	return fn;
    } // fibonacci::next
}; // fibonacci

_Task Worker {
    fibonacci &f1, &f2;
    int n1, n2;
    
    void main() {
	yield( rand() % 10 );
	n1 = f1.next();
	yield( rand() % 10 );
	n2 = f2.next();
	osacquire( cout ) << "task " << &uThisTask() << " " << n1 << " " << n2 << endl;
    } // Worker::main
  public:
    Worker( fibonacci &f1, fibonacci &f2 ) : f1( f1 ), f2( f2 ) {
    } // Worker::Worker
}; // Worker

void uMain::main() {
    const int NoOfWorkers = 10;
    fibonacci f1, f2;					// create fibonacci generator

    srand( getpid() );
    osacquire( cout ) << "Fibonacci Numbers" << endl;

    Worker *workers = new Worker[NoOfWorkers]( f1, f2 );
    delete [] workers;

    osacquire( cout ) << "successful completion" << endl;
} // uMain::main

// Local Variables: //
// compile-command: "u++ FibPar.cc" //
// End: //
