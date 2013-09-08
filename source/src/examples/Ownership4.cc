//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 2006
// 
// Ownership4.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Feb 24 17:30:59 2006
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Feb 24 17:51:16 2006
// Update Count     : 12
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Coroutine C;						// foward declaration
_Cormonitor CM;

_Monitor M {
    CM &c;
  public:
    M( CM &c ) : c( c ) {}
    void mem();
}; // M

_Coroutine C {
    M &m;

    void main() {
	m.mem();
    } // C::main
  public:
    C( M &m ) : m( m ) {}
    ~C() {
	cout << "Here1" << endl;
    }
    void mem() {
	resume();
    } // C::mem

    void mem2() {
	suspend();
    } // C::mem
}; // C

_Cormonitor CM {
    uCondition c;
  public:
    CM() {}

    void mem() {
	resume();
    } // CM::mem

    void mem2() {
	cout << "Here1.1" << endl;
	c.wait();
    } // CM::mem2
  private:
    void main() {
	cout << "Here2" << endl;
	c.signalBlock();
	_Accept( mem2 );
	cout << "Here3" << endl;
    } // C::main
}; // CM

void M::mem() {
    c.mem();
} // M::mem

_Task T {
    C &c;
  public:
    T( C &c, const char *name ) : c( c ) {
	setName( name );
    } // T::T

    void mem() {}
  private:
    void main() {
	c.mem();
	cout << "Here4" << endl;
    } // T::main
}; // T

void uMain::main() {
    CM cm;
    M m( cm );
    C c( m );
    T &t = *new T( c, "T" );
    cm.mem2();
    cout << "Here5" << endl;
} // uMain::main
