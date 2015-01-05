//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 2011
// 
// Accept.cc -- Check dynamically nested accepting returns to the correct code after the accept clause that accepted the
//              call.
// 
// Author           : Peter A. Buhr
// Created On       : Mon Mar 21 18:16:14 2011
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Sep 26 01:36:03 2012
// Update Count     : 3
// 

#include <iostream>
using namespace std;


_Task T1 {
    int check;
    void main();
  public:
    void X();
    void Y();
    void Z();
};

_Task T0 {
    T1 &t1;
    void main() {
	t1.Y();
    }
  public:
    T0( T1 &t1 ) : t1( t1 ) {}
};

void T1::X() {
    check = 1;
}
void T1::Y() {
    check = 2;
    X();
    check = 2;
}
void T1::Z() {
    check = 3;
    T0 to( *this );
    _Accept( X ) {
	assert( check == 1 );
    } or _Accept( Y ) {
	assert( check == 2 );
    } or _Accept( Z ) {
	assert( check == 3 );
    }
    check = 3;
}
void T1::main() {
    for ( ;; ) {
	_Accept( ~T1 ) {
	    break;
	} or _Accept( X ) {
	    assert( check == 1 );
	} or _Accept( Y ) {
	    assert( check == 2 );
	} or _Accept( Z ) {
	    assert( check == 3 );
	}
    }
}

_Task T2 {
    T1 t1;
    void main() {
	t1.X();
	t1.Y();
	t1.Z();
    }
};

void uMain::main() {
    T1 t1;
    t1.X();
    yield( 5 );
    t1.Y();
    t1.Z();
    T2 t2;
    cout << "successful completion" << endl;
}
