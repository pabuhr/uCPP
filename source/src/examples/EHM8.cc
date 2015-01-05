//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 2003
// 
// EHM8.cc -- 
// 
// Author           : Roy Krischer
// Created On       : Wed Oct  8 22:02:29 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Dec  8 17:37:21 2011
// Update Count     : 74
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

#define ROUNDS  10000
#define NP 8

_Event E1 {};
_Event E2 {};


void one() {
    uAbort( "invalid 1\n" );
}
void two() {
    uAbort( "invalid 2\n" );
}
void three() {
    osacquire( cout ) << "success" << endl;
    exit( EXIT_SUCCESS );
}

_Task fred {
    int id;
  public:
    fred( int id ) : id( id ) {}
    void main() {
	if ( id % 2 ) { 
	    std::set_terminate( one );
	    std::set_unexpected( two );
	} else {
	    std::set_terminate( two );
	    std::set_unexpected( one );
	} // if

	for ( int i = 0; i < ROUNDS; i += 1 ) {
	    yield();
	    if ( id % 2 ) {
		assert( one == std::set_terminate( one ) );
		assert( two == std::set_unexpected( two ) );
	    } else {
		assert( two == std::set_terminate( two ) );
		assert( one == std::set_unexpected( one ) );
	    } // if	    
	} // for
    } // fred::main    
}; // fred


void T1() {
    osacquire( cout ) << "T1" << endl;
    _Throw E2();
}
void T2() {
    osacquire( cout ) << "T2" << endl;
    _Throw E2();
}
void T3() {
    osacquire( cout ) << "T3" << endl;
    _Throw E2();
}

_Task mary {
  public:
    void mem() throw(E2) {
	_Throw E1();
    }
  private:
    void m1() throw(E2) {
	std::set_unexpected( T3 );
	_Throw E1();
    }
    void m2() throw(E2) {
	_Throw E1();
    }
    void main() {
	try {
	    _Accept( mem );
	} catch( uMutexFailure::RendezvousFailure ) {
	    osacquire( cout ) << "mary::main 0 caught uRendezvousFailure" << endl;
	}
	try {
	    m1();
	} catch( E2 ) {
	    osacquire( cout ) << "mary::main 1 caught E2" << endl;
	}
	std::set_unexpected( T1 );
	try {
	    m2();
	} catch( E2 ) {
	    osacquire( cout ) << "mary::main 2 caught E2" << endl;
	}
    }
};

void uMain::main() {
    uProcessor processors[NP - 1] __attribute__(( unused )); // more than one processor
    fred *f[NP];
    int i;

    for ( i = 0; i < NP; i += 1 ) {
	f[i] = new fred( i );
    } // for
    for ( i = 0; i < NP; i += 1 ) {
	delete f[i];
    } // for

    mary m;
    std::set_unexpected( T2 );
    try {
	m.mem();
    } catch( E2 ) {
	osacquire( cout ) << "uMain::main caught E2" << endl;
    }
} // uMain::main
