//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// NBStream.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Wed Apr 28 13:23:14 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Dec  8 17:45:29 2011
// Update Count     : 28
// 

#include <iostream>
using std::cin;
using std::cerr;

int d = 0;						// shared by reader and writer

_Task Reader {
    void main() {
	try {
	    for ( ;; ) {
		cin >> d;				// read number from stdin
	      if ( cin.fail() ) break;
	    } // for
	} catch( ... ) {
	    uAbort( "reader failure" );
	} // try
    } // Reader::main
}; // Reader

_Task Writer {
    void main() {
	try {
	    for ( int i = 0;; i += 1 ) {
	      if ( cin.fail() ) break;
		if ( i % 100 == 0 ) {			// don't print too much
		    cerr << d;				// write number to stderr (no buffering)
		} // if					// if cout is used, it must be flushed with endl
		yield( 1 );
	    } // for
	} catch( ... ) {
	    uAbort( "writer failure" );
	} // try
    } // Writer::main
}; // Writer

void uMain::main() {
    Reader r;
    Writer w;
} // uMain::main

// Local Variables: //
// compile-command: "u++ NBStream.cc" //
// End: //
