//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2016
// 
// ActorFuture.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Dec 19 08:22:37 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Dec 27 08:36:56 2016
// Update Count     : 4
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
#include <string>
using namespace std;
#include <uActor.h>

struct IntMsgV : public uActor::Message {
    int val;
    IntMsgV( int val ) : Message( uActor::Delete ), val( val ) {}
}; // IntMsg

struct IntMsg : public uActor::FutureMessage< int > {
    int val;
    IntMsg( int val ) : FutureMessage( uActor::Delete ), val( val ) {}
}; // IntMsg

struct StrMsg : public uActor::FutureMessage< string > {
    string val;
    StrMsg( string val ) : FutureMessage( uActor::Delete ), val( val ) {}
}; // StrMsg

struct IntStrMsg : public uActor::FutureMessage< int > {
    string val;
    IntStrMsg( string val ) : FutureMessage( uActor::Delete ), val( val ) {}
}; // StrMsg

struct DoubleMsg : public uActor::FutureMessage< double > {
    double val;
    DoubleMsg( double val ) : FutureMessage( uActor::Delete ), val( val ) {}
}; // DoubleMsg


_Actor Actor {
    Allocation receive( uActor::Message &msg ) {
	Case( IntMsgV, msg ) {				// tell message
	    osacquire( cout ) << "Actor " << msg_t->val << endl;
	} else Case( IntMsg, msg ) {			// ask messages
	    osacquire( cout ) << "Actor " << msg_t->val << endl;
	    msg_t->delivery( 7 );
	} else Case( StrMsg, msg ) {
	    osacquire( cout ) << "Actor " << msg_t->val << endl;
	    msg_t->delivery( "XYZ" );
	} else Case( IntStrMsg, msg ) {
	    osacquire( cout ) << "Actor " << msg_t->val << endl;
	    msg_t->delivery( 12 );
	} else Case( StopMsg, msg ) {
	    return Delete;				// delete actor

	// error cases
	} else Case( UnhandledMsg, msg ) {		// receiver complained
	    abort( "sent unknown message to %p", msg.sender );
	} else Case( uActor::ReplyMsg, msg ) {		// unknown future message
	    _Throw uActor::Unhandled( msg_t );		// complain in future
	    // msg_t->delivery( new uActor::Unhandled( msg_t ) ); // alternative
	} else {					// unknown void message
	    *msg.sender | uActor::unhandledMsg;		// complain to sender
	} // Case

	return Nodelete;				// reuse actor
    } // Actor::receive
}; // Actor

int main() {
    enum { Times = 5 };
    Future_ISM< int > fi[Times];
    Future_ISM< string > fs[Times];
    Actor * actor = new Actor;

    for ( int i = 0; i < Times; i += 1 ) {		// tell messages
    	*actor | *new IntMsgV( 2 );			// ignore future
    	*actor | *new IntMsg( 3 );
    	*actor | *new StrMsg( "ABC" );
    } // for

    for ( int i = 0; i < Times; i += 1 ) {		// ask messages
	fi[i] = *actor || *new IntMsg( 4 );		// store future
	fs[i] = *actor || *new StrMsg( "DEF" );
    } // for
    for ( int i = 0; i < Times; i += 1 ) {		// access future values
	osacquire( cout ) << "futures " << fi[i]() << " " << fs[i]() << endl;
    } // for

    for ( int i = 0; i < Times; i += 1 ) {		// ask messages
    	fi[i] = *actor || *new IntStrMsg( "HIJ" );	// store future
    } // for
    for ( int i = 0; i < Times; i += 1 ) {		// access future values
    	osacquire( cout ) << "futures " << fi[i]() << endl;
    } // for

    try {						// unhandled future message
	(*actor || *new DoubleMsg( 3.5 ))();		// throws exception from future
    } catch( uActor::Unhandled &unhdle ) {
    	cout << "Unhandled ask message " << unhdle.msg << " to actor " << actor << endl;
    } // try

    *actor | uActor::stopMsg;				// terminate actor
    uActor::stop();					// wait for all actors to terminate
} // main

// Local Variables: //
// compile-command: "u++-work -g -O2 -multi ActorFuture.cc" //
// End: //
