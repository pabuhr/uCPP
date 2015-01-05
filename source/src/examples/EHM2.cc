//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1998
// 
// EHM2.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Tue Oct 27 21:24:48 1998
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Dec  8 17:36:06 2011
// Update Count     : 40
// 


_Event xxx {
  public:
    uBaseTask *tid;
    xxx( uBaseTask *tid ) : tid(tid) {}
};

_Event yyy {
  public:
    uBaseTask *tid;
    yyy( uBaseTask *tid ) : tid(tid) {}
};


_Task fred {
    void r( int i ) {
	if ( i == 0 ) {
	    _Throw xxx( &uThisTask() );
	} else {
	    r( i - 1 );
	} // if
    } // fred::r

    void s( int i ) {
	if ( i == 0 ) {
	    try {
		r( 5 );
	    } catch( xxx e ) {
		assert( e.tid == &uThisTask() );
		_Throw;
	    }
	} else {
	    s( i - 1 );
	} // if
    } // fred::s

    void t( int i ) {
	if ( i == 0 ) {
	    try {
		s( 5 );
	    } catch( xxx e ) {
		assert( e.tid == &uThisTask() );
		_Throw yyy( &uThisTask() );
	    }
	} else {
	    t( i - 1 );
	} // if
    } // fred::t
    
    void main() {
	for ( int i = 0; i < 5000; i += 1 ) {
//	for ( int i = 0; i < 5; i += 1 ) {
	    try {
		t( 5 );
	    } catch( yyy e ) {
		assert( e.tid == &uThisTask() );
	    } // try
	} // for
    } // fred::main
}; // fred

void uMain::main() {
    uProcessor processors[3] __attribute__(( unused ));	// more than one processor
    fred f[4] __attribute__(( unused ));
} // uMain::main
