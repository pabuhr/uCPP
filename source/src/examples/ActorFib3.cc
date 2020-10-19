// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2020
// 
// ActorFib3.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Wed Feb 12 16:20:35 2020
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Sep 30 21:22:09 2020
// Update Count     : 12
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

struct FibMsg : public uActor::Message {
	long int fn;
	FibMsg() : Message( uActor::Delete ) {}
};
struct NextMsg : public uActor::Message {};

_CorActor Fib {
	FibMsg * fibMsg;
	long int * fn;										// pointer into return message
	NextMsg * msg;

	Allocation receive( Message & msg ) {
		Case( NextMsg, msg ) {
			Fib::msg = msg_d;
			fibMsg = new FibMsg;
			fn = &fibMsg->fn;							// optimization
			resume();
		} else Case( StopMsg, msg ) return Finished;
		return Nodelete;
	} // Fib::receive

	void main() {
		long int fn1, fn2;

		*fn = 0; fn1 = *fn;								// fib(0) => 0
		*msg->sender() | *fibMsg;						// return fn
		suspend();

		*fn = 1; fn2 = fn1; fn1 = *fn;					// fib(1) => 1
		*msg->sender() | *fibMsg;						// return fn
		suspend();

		for ( ;; ) {
			*fn = fn1 + fn2; fn2 = fn1; fn1 = *fn;		// fib(n) => fib(n-1) + fib(n-2)
			*msg->sender() | *fibMsg;					// return fn
			suspend();
		} // for
	} // Fib::main
}; // Fib

int Times = 10;											// default values

_Actor Generator {
	int i = 0;
	Fib fib;
	NextMsg nextMsg;

	void preStart() {
		fib | nextMsg;
	} // Generator::preStart

	Allocation receive( Message & msg ) {
		if ( i < Times ) {
			Case( FibMsg, msg ) {
				cout << msg_d->fn << endl;
				fib | nextMsg;
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
		  case 1:										// use defaults
			break;
		  default:
			throw 1;
		} // switch
	} catch( ... ) {
		cout << "Usage: " << argv[0] << " [ numbers (> 0) ]" << endl;
		exit( EXIT_FAILURE );
	} // try

	uActorStart();										// start actor system
	Generator fib1, fib2;
	uActorStop();										// wait for all actors to terminate
} // main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work -g -O2 -multi ActorFib3.cc" //
// End: //
