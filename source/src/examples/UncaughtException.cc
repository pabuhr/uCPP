//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 2010
// 
// UncaughtException.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sun Jul 18 11:14:42 2010
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 18 11:14:47 2010
// Update Count     : 1
// 

#include <iostream>
using namespace std;

_Task T1 {
    void main() {
	for( ;; ) {
	    _Accept( ~T1 ) break;
	    try {
		throw 1;
	    } catch( int ) {
	    }
	}
    }
};

_Task T2 {
    void main() {
	for( ;; ) {
	    _Accept( ~T2 ) break;
	    assert( ! std::uncaught_exception() );
	    yield();
	}
    }
};

void uMain::main() {
    T1 t1;
    T2 t2;
    uThisTask().uSleep( uDuration( 5 ) );
    cout << "Successful completion" << endl;
}
