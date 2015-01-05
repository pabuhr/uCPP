//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// File.cc -- Print multiple copies of the same file to standard output
// 
// Author           : Peter A. Buhr
// Created On       : Tue Jan  7 08:44:56 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Dec  8 17:43:07 2011
// Update Count     : 44
// 

#include <uFile.h>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

_Task Copier {
	uFile &input;

	void main() {
		uFile::FileAccess in( input, O_RDONLY );
		int count;
		char buf[1];

		for ( int i = 0;; i += 1 ) {					// copy in-file to out-file
			count = in.read( buf, sizeof( buf ) );
		  if ( count == 0 ) break;						// eof ?
			cout << buf[0];
			if ( i % 20 == 0 ) yield();
		} // for
	} // Copier::main
  public:
	Copier( uFile &in ) : input( in ) {
	} // Copier::Copier
}; // Copier

void uMain::main() {
	switch ( argc ) {
	  case 2:
		break;
	  default:
		cerr << "Usage: " << argv[0] << " input-file" << std::endl;
		exit( EXIT_FAILURE );
	} // switch

	uFile input( argv[1] );								// connect with UNIX files
	{
		Copier c1( input ), c2( input );
	}
} // uMain::main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ File.cc" //
// End: //
