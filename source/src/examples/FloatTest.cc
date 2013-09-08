//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Robert Denda 1996
// 
// FloatTest.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Wed May 11 17:30:51 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Jun 29 17:09:26 2012
// Update Count     : 77
// 

#include <cmath>
#ifndef X_EPS
#define X_EPS 1.0E-8
#endif
#include <iostream>
using std::cout;
using std::endl;

unsigned int uDefaultPreemption() {
    return 1;						// set time-slice to one millisecond
} // uDefaultPreemption


const double Range = 10000.0;


_Task Tester {
    double result;
    int TaskId;

    void main() {
	register double d;	     
	uFloatingPointContext context;			// save/restore floating point registers during context switch

	// Each task increments a counter through a range of values and calculates a trigonometric identity of each
	// value in the range. There should be numerous context switches while performing this calculation. If the
	// floating point registers are not saved properly for each task, the calculations interfere producing erroneous
	// results.

	for ( d = TaskId * Range; d < (TaskId + 1) * Range; d += 1.0 ) {
	    yield();
	    if ( fabs( sqrt( pow( sin( d ), 2.0 ) + pow( cos( d ), 2.0 ) ) - 1.0 ) > X_EPS ) { // self-check 
		uAbort( "invalid result during Tester[%d]:%6f", TaskId, d );
	    } // if
	    if ( d < TaskId * Range || d >= (TaskId + 1) * Range ) { // self-check 
		uAbort( "invalid result during Tester[%d]:%6f", TaskId, d );
	    } // if
	} // for
	result = d;
    } // Test::main
  public:
    Tester( int TaskId ) : TaskId( TaskId ) {}
    double Result() { return result; }
}; // Tester


void uMain::main() {
    const int numTesters = 10;
    Tester *testers[numTesters];
    int i;
    uFloatingPointContext context;			// save/restore floating point registers during a context switch

    for ( i = 0; i < numTesters; i += 1 ) {		// create tasks
	testers[i] = new Tester(i);
	yield();
    } // for

    for ( i = 0; i < numTesters; i += 1 ) {		// recover results and delete tasks
	double result = testers[i]->Result();
	if ( result != (i + 1) * Range ) {		// check result
	    uAbort( "invalid result in Tester[%d]:%6f", i, result );
	} // if
	delete testers[i];
    } // for

    cout << "successful completion" << endl;
} // uMain::main
    
// Local Variables: //
// compile-command: "u++ -O2 FloatTest.cc" //
// End: //
