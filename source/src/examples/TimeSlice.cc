//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// TimeSlice.cc -- test time slice
// 
// Author           : Peter A. Buhr
// Created On       : Mon Apr 26 11:04:37 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Apr  6 09:05:05 2012
// Update Count     : 33
// 


unsigned int uDefaultPreemption() {
    return 1;
} // uDefaultPreemption

enum { NoOfTimes = 1000 };
volatile int x = 0, y = 1;

_Task T1 {
    void main() {
	for ( ;; ) {
	  if ( x == NoOfTimes ) break;
	    if ( x < y ) x += 1;
	} // for
    } // T1::main
}; // T1

_Task T2 {
    void main() {
	for ( ;; ) {
	  if ( y == NoOfTimes ) break;
	    if ( y == x ) y += 1;
	} // for
    } // T2::main
}; // T2

void uMain::main() {
    T1 t1;
    T2 t2;
} // uMain::main

// Local Variables: //
// compile-command: "u++ TimeSlice.cc" //
// End: //
