//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// ClientUNIXSTREAM.cc -- Client for UNIX/stream socket test. Client reads from standard input, writes the data to the
//     server, reads the data from the server, and writes that data to standard output.
// 
// Author           : Peter A. Buhr
// Created On       : Tue Jan  7 08:42:32 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Dec  8 17:42:23 2011
// Update Count     : 106
// 

#include <uSocket.h>
#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::osacquire;
using std::endl;

// minimum buffer size is 2, 1 character and string terminator, '\0'
enum { BufferSize = 65 };
const char EOD = '\377';
const char EOT = '\376';
unsigned int rcnt = 0, wcnt = 0;

_Task Reader {
	uSocketClient &client;

	void main() {
		char buf[BufferSize];
		int len;

		for ( ;; ) {
			len = client.read( buf, sizeof(buf) );
			// osacquire( cerr ) << "reader read len:" << len << endl;
		  if ( len == 0 ) uAbort( "client %d : EOF ecountered without EOD", getpid() );
			rcnt += len;
			// The EOD character can be piggy-backed onto the end of the message.
		  if ( buf[len - 1] == EOD ) {
			  rcnt -= 1;								// do not count the EOD
			  cout.write( buf, len - 1 );				// do not write the EOD
			  client.write( &EOT, sizeof(EOT) );		// indicate EOD received
			  break;
			} // exit
			cout.write( buf, len );
		} // for
	} // Reader::main
  public:
	Reader( uSocketClient &client ) : client ( client ) {
	} // Reader::Reader
}; // Reader

_Task Writer {
	uSocketClient &client;

	void main() {
		char buf[BufferSize];

		for ( ;; ) {
			cin.get( buf, sizeof(buf), '\0' );			// leave room for string terminator
		  if ( buf[0] == '\0' ) break;
			int len = strlen( buf );
			// osacquire( cerr ) << "writer read len:" << len << endl;
			wcnt += len;
			client.write( buf, len );
		} // for
		client.write( &EOD, sizeof(EOD) );
	} // Writer::main
  public:
	Writer( uSocketClient &client ) : client( client ) {
	} // Writer::Writer
}; // Writer

void uMain::main() {
	switch ( argc ) {
	  case 2:
		break;
	  default:
		cerr << "Usage: " << argv[0] << " socket-name" << endl;
		exit( EXIT_FAILURE );
	} // switch

	// remove tie due to race between cin flush and cout write by writer and reader tasks
	cin.tie( NULL );
	uSocketClient client( argv[1] );					// connection to server
	{
		Reader rd( client );							// emit worker to read from server and write to output
		Writer wr( client );							// emit worker to read from input and write to server
	}
	if ( wcnt != rcnt ) {
		uAbort( "Error: client not all data transfered, wcnt:%d rcnt:%d", wcnt, rcnt );
	} // if
} // uMain::main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work ClientUNIXSTREAM.cc -o Client" //
// End: //
