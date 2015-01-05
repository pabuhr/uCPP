//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1999
// 
// ServerINETDGRAM.cc -- Server for INET/datagram socket test. Server reads data from multiple clients. The server reads
//     the data from the client and writes it back.
// 
// Author           : Peter A. Buhr
// Created On       : Thu Apr 29 16:02:50 1999
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Dec  8 17:48:21 2011
// Update Count     : 44
// 

#include <uSocket.h>
#include <iostream>
using std::cout;
using std::cerr;
using std::osacquire;
using std::endl;

enum { BufferSize = 8 * 1024 };

_Task Reader {
	uSocketServer &server;

	void main() {
		uDuration timeout( 20, 0 );						// timeout for read
		char buf[BufferSize];
		int len;

		try {
			for ( ;; ) {
				len = server.recvfrom( buf, sizeof(buf), 0, &timeout );
				// osacquire( cerr ) << "reader read len:" << len << endl;
			  if ( len == 0 ) uAbort( "server %d : EOF ecountered before timeout", getpid() );
				server.sendto( buf, len );				// write byte back to client
			} // for
		} catch( uSocketServer::ReadTimeout ) {
		} // try
	} // Reader::main
  public:
	Reader( uSocketServer &server ) : server( server ) {
	} // Reader::Reader
}; // Reader

void uMain::main() {
	switch ( argc ) {
	  case 1:
		break;
	  default:
		cerr << "Usage: " << argv[0] << endl;
		exit( EXIT_FAILURE );
	} // switch

	short unsigned int port;
	uSocketServer server( &port, SOCK_DGRAM );			// create and bind a server socket to free port

	cout << port << endl;								// print out free port for clients
	{
		Reader rd( server );							// execute until reader times out
	}
} // uMain

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work -o Server ServerINETDGRAM.cc" //
// End: //
