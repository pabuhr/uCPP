//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 2002
// 
// EHM7.cc -- 
// 
// Author           : Roy Krischer
// Created On       : Sun Nov 24 12:42:34 2002
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Aug 21 07:45:02 2008
// Update Count     : 27
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Event fred {
  public:
    int k;
    fred ( int k ) : k(k) {}
};

_Task mary {
  public:
    void main();
};

_Task john {
  public:
    void main();
};

mary m;
john j;

void mary :: main() {
    _Resume fred( 42 ) _At j;
    try {
		_Enable {
			for ( int i = 0; i < 200; i+= 1 ) yield();
		} // _Enable
    } catch ( j.fred f ) {
		assert( &j == f.getOriginalThrower() );
		assert( &j == &f.source() );
		assert( f.k == 84 ); 
		osacquire( cout ) << "mary catches exception from john: " << f.k << endl;
	    for ( int i = 0; i < 200; i+= 1 )
			yield();
    } // try
}
    
void john :: main() {
    _Resume fred( 84 ) _At m;
    try {
		_Enable {
			for ( int i = 0; i < 200; i+= 1 ) yield();
		} // _Enable
    } catch ( m.fred f ) {
		assert( &m == f.getOriginalThrower() );
		assert( &m == &f.source() );
		assert( f.k == 42 ); 
		osacquire( cout ) << "john catches exception from mary: " << f.k << endl
			  << "mary m's address: " << (void *)&m << " exception binding: " << f.getOriginalThrower()
			  << " exception Src: " << (void *)&f.source() << endl;
	 
	    for ( int i = 0; i < 200; i+= 1 ) yield();
    } // try
}

void uMain::main() {
	for ( int i = 0; i < 200; i+= 1 ) yield();
}

// Local Variables: //
// tab-width: 4 //
// End: //
