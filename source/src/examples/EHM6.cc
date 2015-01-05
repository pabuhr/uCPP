//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1998
// 
// EHM6.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Tue Oct 27 21:24:48 1998
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Aug 14 09:14:27 2009
// Update Count     : 27
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Event R1 {
  public:
	int &i; char &c;
	R1( int &i, char &c ) : i( i ), c( c ) {}
};

_Event R2 {};


class fred {
  public:
	void f( int x, char y ) {
		osacquire( cout ) << "enter f, x:" << x << " y:" << y << endl;
		_Resume R2();
		osacquire( cout ) << "exit f, x:" << x << " y:" << y << endl;
	}
	void g( int &x, char &y ) {
		osacquire( cout ) << "enter g, x:" << x << " y:" << y << endl;
		_Resume R1( x, y );
		osacquire( cout ) << "exit g, x:" << x << " y:" << y << endl;
	}
};


void uMain::main() {
	fred ff, gg;

	try {
		int x = 0;
		char y = 'a';

		ff.g( x, y );
		osacquire( cout ) << "try<R1,rtn1> x:" << x << " y:" << y << endl;

		// Check multiple handlers, only one is used.
		try {
			gg.f( x, y );
			osacquire( cout ) << "try<R2,rtn2>, x:" << x << " y:" << y << endl;
		} _CatchResume( R1 ) {
		} _CatchResume( gg.R2 ) ( int x, char y ) {
			osacquire( cout ) << "rtn2, i:" << x << " c:" << y << endl;
			x = 2; y = 'c';									// change x, y
		} // try
		try {								// R1 empty handler
			ff.g( x, y );
			osacquire( cout ) << "try<R1>, x:" << x << " y:" << y << endl;
		} _CatchResume( R1 ) {
		} _CatchResume( gg.R2 ) ( int x, char y ) {
			osacquire( cout ) << "rtn2, i:" << x << " c:" << y << endl;
			x = 2; y = 'c';									// change x, y
		} // try
	} _CatchResume( R1 &r ) {
		osacquire( cout ) << "rtn1" << endl;
		r.i = 1; r.c = 'b';
	} // try
}

// Local Variables: //
// tab-width: 4 //
// End: //
