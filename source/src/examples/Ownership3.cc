//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 2006
// 
// Ownership3.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Wed Feb 22 23:20:03 2006
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Feb 24 15:36:56 2006
// Update Count     : 11
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Coroutine C;						// foward declaration

_Monitor M {
  public:
    void mem( C &c );
}; // M

_Coroutine C {
    M &m;

    void main() {
	m.mem( *this );
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

void M::mem( C &c ) {
    c.mem2();
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
	cout << "Here2" << endl;
	_Accept( mem );
	_Accept( mem );
    } // T::main
}; // T

void uMain::main() {
    M m;
    C c( m );
    T &t = *new T( c, "T" );
    t.mem();
    cout << "Here3" << endl;
} // uMain::main
