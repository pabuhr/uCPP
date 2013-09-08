//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uCalibrate.cc -- Calibrate the number of iterations of a set piece of code to produce a 100 microsecond delay.
// 
// Author           : Peter A. Buhr
// Created On       : Fri Aug 16 14:12:08 1991
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Feb  5 16:16:40 2011
// Update Count     : 37
// 

#include <iostream>
using std::cout;
using std::endl;

#include "Time.h"

#define TIMES 5000000000LL				// cannot be larger or overflow occurs

void uMain::main() {
    unsigned long long int StartTime, EndTime;
    
    StartTime = Time();
    for ( volatile unsigned long long int i = 1; i <= TIMES; i += 1 ) {
    } // for
    EndTime = Time();

    cout << "#define ITERATIONS_FOR_100USECS " << 100000LL * TIMES / ( EndTime - StartTime ) << endl;
} // uMain::main

// Local Variables: //
// compile-command: "u++ uCalibrate.cc" //
// End: //
