//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// SpinLock.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Feb 13 15:47:48 1995
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Sep  3 17:11:18 2008
// Update Count     : 17
// 


unsigned int uDefaultPreemption() {
    return 1;
} // uDefaultPreemption

void CriticalSection() {
    static volatile uBaseTask *CurrTid;			// current task id
    CurrTid = &uThisTask();				// address of current task
    
    for ( int i = 1; i <= 100; i += 1 ) {		// delay
	// perform critical section operation
	if ( CurrTid != &uThisTask() ) {		// check for mutual exclusion violation
	    uAbort( "interference" );
	} // if
    } // for
} // CriticalSection   

uSpinLock Lock;

_Task Tester {
    void main() {
	for ( int i = 1; i <= 10000000; i += 1 ) {
	    ::Lock.acquire();
	    CriticalSection();				// critical section
	    ::Lock.release();
	} // for
    } // main
  public:
    Tester() {}
}; // Tester
    
void uMain::main() {
    uProcessor processor[1] __attribute__(( unused ));	// more than one processor
    Tester t[10];
} // main

// Local Variables: //
// compile-command: "../../bin/u++ -multi -g SpinLock.cc" //
// End: //
