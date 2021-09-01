// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2020
// 
// ActorFib4.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Apr 30 08:14:04 2021
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue May 25 10:38:12 2021
// Update Count     : 15
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

struct FibMsg : public uActor::Message { long int fn; }; // Nodelete

_CorActor Fib {
	long int * fn;
	FibMsg * msg;

	Allocation receive( Message & msg ) {
		Case( FibMsg, msg ) {
			Fib::msg = msg_d;							// coroutine communication
			fn = &msg_d->fn;							// compute answer in message
			resume();
		} else Case( StopMsg, msg ) return Finished;
		return Nodelete;
	} // Fib::receive

	void main() {
		long int fn1, fn2;

		*fn = 0; fn1 = *fn;								// fib(0) => 0
		*msg->sender() | *msg;							// return fn
		suspend();

		*fn = 1; fn2 = fn1; fn1 = *fn;					// fib(1) => 1
		*msg->sender() | *msg;							// return fn
		suspend();

		for ( ;; ) {
			*fn = fn1 + fn2; fn2 = fn1; fn1 = *fn;		// fib(n) => fib(n-1) + fib(n-2)
			*msg->sender() | *msg;						// return fn
			suspend();
		} // for
	} // Fib::main
}; // Fib

int Times = 10;											// default values

_Actor Generator {
	int i = 0;
	Fib fib;
	FibMsg fibMsg;

	void preStart() {
		fib | fibMsg;									// kick start generator
	} // Generator::preStart

	Allocation receive( Message & msg ) {
		if ( i < Times ) {
			Case( FibMsg, msg ) {
				cout << msg_d->fn << endl;
				fib | fibMsg;
			} // Case
			i += 1;
			return Nodelete;
		} else {
			fib | stopMsg;
			return Finished;
		} // if
	} // Generator::receive
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

	uActor::start();									// start actor system
	Generator fib1, fib2;
	uActor::stop();										// wait for all actors to terminate
} // main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work -g -O2 -multi ActorFib4.cc" //
// End: //
