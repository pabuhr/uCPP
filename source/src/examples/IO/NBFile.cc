//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// NBFile.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Tue Apr 27 20:39:18 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Dec  8 10:19:06 2009
// Update Count     : 24
// 

#include <uFile.h>
#include <iostream>
using std::cin;

char ch = '0';						// shared by reader and writer

_Task Reader {
    void main() {
	char tch;

	for ( ;; ) {
	    cin >> tch;					// read number from stdin
	    if ( tch != '\n' ) ch = tch;
	  if ( cin.eof() ) break;
	} // for
    } // Reader::main
}; // Reader

_Task Writer {
    void main() {
	uFile::FileAccess output( "xxx", O_WRONLY | O_CREAT | O_TRUNC, 0666 );
	int i;

	for ( i = 0;; i += 1 ) {
	  if ( cin.eof() ) break;
	    output.write( &ch, 1 );			// write number to stdout
	    yield( 1 );
	} // for
    } // Writer::main
}; // Writer

void uMain::main() {
    Reader reader;
    Writer writer;
} // uMain::main

// Local Variables: //
// compile-command: "u++ NBFile.cc" //
// End: //
