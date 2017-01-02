//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2002
// 
// EHM7.cc -- 
// 
// Author           : Roy Krischer
// Created On       : Sun Nov 24 12:42:34 2002
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Dec 19 08:44:36 2016
// Update Count     : 28
//
// This  library is free  software; you  can redistribute  it and/or  modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software  Foundation; either  version 2.1 of  the License, or  (at your
// option) any later version.
// 
// This library is distributed in the  hope that it will be useful, but WITHOUT
// ANY  WARRANTY;  without even  the  implied  warranty  of MERCHANTABILITY  or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
// 
// You should  have received a  copy of the  GNU Lesser General  Public License
// along  with this library.
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
