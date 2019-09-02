// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2016
// 
// ActorPingPong.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Dec 19 08:24:00 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Feb  5 12:52:44 2019
// Update Count     : 14
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
using namespace std;
#include <uActor.h>

#ifdef NOOUTPUT
#define PRT( stmt )
#else
#define PRT( stmt ) stmt
#endif // NOOUTPUT

struct PingMsg : public uActor::Message {} pingMsg;
struct PongMsg : public uActor::Message {} pongMsg;

_Actor Ping {
    int cycle = 0, cycles;

    Allocation receive( Message &msg ) {
	Case( PongMsg, msg ) {				// determine message kind
	    if ( cycle < cycles ) {			// keep cycling ?
		cycle += 1;
		PRT( cout << "ping "; )
		msg.sender->tell( pingMsg, this );	// return to sender
	    } else {
		PRT( cout << "ping stop" << endl; )	// stop cycling
		msg.sender->tell( stopMsg, this );	// return to sender
		return Finished;
	    } // if
	} // Case
	return Nodelete;				// reuse actor
    } // Ping::receive
  public:
    Ping( int cycles = 10 ) : cycles( cycles ) {}
}; // Ping

_Actor Pong {
    Allocation receive( Message &msg ) {
	Case( PingMsg, msg ) {				// determine message kind
	    PRT( cout << "pong" << endl; )
	    msg.sender->tell( pongMsg, this );		// respond to sender
	} else Case( StopMsg, msg ) {			// stop cycling
	    PRT( cout << "pong stop" << endl; )
	    return Finished;
	} // Case
	return Nodelete;				// reuse actor
    } // Pong::receive
}; // Pong

int main( int argc, char *argv[] ) {
    int Cycles = 10;					// default values

    try {
	switch ( argc ) {
	  case 2:
	    Cycles = stoi( argv[1] );
	    if ( Cycles < 1 ) throw 1;
	  case 1:					// use defaults
	    break;
	  default:
	    throw 1;
	} // switch
    } catch( ... ) {
	cout << "Usage: " << argv[0] << " [ cycles (> 0) ]" << endl;
	exit( EXIT_SUCCESS );
    } // try

    uActorStart();					// start actor system
    Ping ping( Cycles );
    Pong pong;
    ping.tell( pongMsg, &pong );			// start cycling
    uActorStop();					// wait for all actors to terminate
} // main

// Local Variables: //
// compile-command: "u++-work -g -O2 -multi ActorPingPong.cc" //
// End: //
