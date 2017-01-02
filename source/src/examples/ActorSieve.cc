//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2016
// 
// ActorSieve.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Dec 19 08:26:15 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Dec 27 08:33:51 2016
// Update Count     : 9
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

unsigned int uDefaultActorThreads() { return 1; }	// 1 => user thread
unsigned int uDefaultActorProcessors() { return 0; }	// 0 => 1 kernel thread from user cluster

struct Number : public uActor::Message {
    int n;
    Number( int n ) : Message( uActor::Delete ), n( n ) {}
}; // Number

struct DummyMsg : public uActor::Message {} dummyMsg;


_Actor Filter {
    const int myprime;
    uActor *root, *next = nullptr;

    void preStart() {
	*root | *new Number( myprime );			// report prime
    } // Filter::preStart

    Allocation receive( Message &msg ) {
	Case( Number, msg ) {				// determine message kind
	    if ( msg_t->n % myprime != 0 ) {
		if ( ! next ) {
		    next = new Filter( msg_t->n, root );
		} else {
		    *next | *new Number( msg_t->n );
		} // if
	    } // if
	} else Case( StopMsg, msg ) {
	    (next ? *next : *root) | msg;		// propagate stop or stop root at list end
	    return Delete;				// delete actor
	} // Case
	return Nodelete;				// reuse actor
    } // Filter::receive
  public:
    Filter( const int myprime, uActor *root ) : myprime( myprime ), root( root ) {}
}; // Filter

_Actor Sieve {
    const int Max;

    void preStart() {
	uActor *first = new Filter( 2, this );		// only even prime
	for ( int i = 3; i < Max; i += 2 ) {		// primes are odd
	    *first | *new Number( i );			// check for prime
	} // for
	*first | stopMsg;				// end number list with stop
    } // Sieve::preStart

    Allocation receive( Message &msg ) {
	Case( Number, msg ) {				// determine message kind
	    osacquire( cout ) << msg_t->n << endl;	// print reported prime
	} else Case( StopMsg, msg ) {
	    osacquire( cout ) << "all done" << endl;
	    return Delete;				// delete actor
	} // Case
	return Nodelete;				// reuse actor
    } // Sieve::receive
  public:
    Sieve( const int Max ) : Max( Max ) {}
}; // Sieve

void uMain::main() {
    int Max = 30;

    try {
	switch ( argc ) {
	  case 2:
	    Max = stoi( argv[1] );
	    if ( Max < 1 ) throw 1;
	  case 1:					// use defaults
	    break;
	  default:
	    throw 1;
	} // switch
    } catch( ... ) {
	cout << "Usage: " << argv[0] << " [ primes-up-to (> 0) ]" << endl;
	exit( 1 );
    } // try

    new Sieve( Max );
    uActor::stop();					// wait for all actors to terminate
} // uMain::main

// Local Variables: //
// compile-command: "u++-work -g -O2 -multi ActorSieve.cc" //
// End: //
