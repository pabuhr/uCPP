// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2016
// 
// ActorFib2.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Dec 19 08:21:34 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Jan  6 08:38:44 2020
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
using namespace std;
#include <uActor.h>

struct FibMsg : public uActor::Message { long int fn; };
struct NextMsg : public uActor::Message {
    FibMsg * fibMsg;
    NextMsg( FibMsg * fibMsg ) : fibMsg(fibMsg) {}
};

_Actor Fib {
    long long int fn1, fn2;

    Allocation f0( Message & msg ) {
	Case( NextMsg, msg ) {				// fib(0) => 0
	    long int & fn = msg_d->fibMsg->fn;		// optimization
	    fn = 0; fn1 = fn;
	    *msg.sender | *msg_d->fibMsg;		// return fn
	    become( &Fib::f1 );				// change state
	} else Case( StopMsg, msg ) return Finished;
	return Nodelete;
    } // Fib::receive

    Allocation f1( Message & msg ) {
	Case( NextMsg, msg ) {				// fib(1) => 1
	    long int & fn = msg_d->fibMsg->fn;		// optimization
	    fn = 1; fn2 = fn1; fn1 = fn;
	    *msg.sender | *msg_d->fibMsg;		// return fn
	    become( &Fib::fn );				// change state
	} else Case( StopMsg, msg ) return Finished;
	return Nodelete;
    } // Fib::f1

    Allocation fn( Message & msg ) {
	Case( NextMsg, msg ) {				// fib(n) => fib(n-1) + fib(n-2)
	    long int & fn = msg_d->fibMsg->fn;		// optimization
	    fn = fn1 + fn2; fn2 = fn1; fn1 = fn;
	    *msg.sender | *msg_d->fibMsg;		// return fn
	} else Case( StopMsg, msg ) return Finished;
	return Nodelete;
    } // Fib::fn
  public:
    Fib() { become( &Fib::f0 ); }			// reset receive
}; // Fib

int Times = 10;						// default values

_Actor Generator {
    int i = 0;
    Fib fib;
    FibMsg fibMsg;
    NextMsg nextMsg = { &fibMsg };

    void preStart() {
	fib.tell( nextMsg, this );
    } // Generator::preStart

    Allocation receive( Message & msg ) {
	if ( i < Times ) {
	    Case( FibMsg, msg ) {
		cout << msg_d->fn << endl;
		fib.tell( nextMsg, this );
	    } // Case
	    i += 1;
	    return Nodelete;
	} else {
	    fib | stopMsg;
	    return Finished;
	} // if
    } // Fib::receive
}; // Generator

int main( int argc, char * argv[] ) {
    try {
	switch ( argc ) {
	  case 2:
	    Times = stoi( argv[1] );
	    if ( Times < 1 ) throw 1;
	  case 1:					// use defaults
	    break;
	  default:
	    throw 1;
	} // switch
    } catch( ... ) {
	cout << "Usage: " << argv[0] << " [ numbers (> 0) ]" << endl;
	exit( EXIT_FAILURE );
    } // try

    uActorStart();					// start actor system
    Generator fib1, fib2;
    uActorStop();					// wait for all actors to terminate
} // main

// Local Variables: //
// compile-command: "u++-work -g -O2 -multi ActorFib2.cc" //
// End: //
