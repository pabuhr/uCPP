//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2017
// 
// ActorChameneos.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sun Jan  8 23:06:40 2017
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Jan 28 09:57:32 2017
// Update Count     : 2
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

// http://cedric.cnam.fr/PUBLIS/RC474.pdf

#ifdef NOOUTPUT						// disable printing for experiments
#define PRT( stmt )
#else
#define PRT( stmt ) stmt
#endif // NOOUTPUT

unsigned int uDefaultActorThreads() { return 1; }
unsigned int uDefaultActorProcessors() { return 0; }


enum Colour { BLUE, RED, YELLOW, NoOfColours };
static const char * ColourNames[NoOfColours] = { "BLUE", "RED", "YELLOW" };
inline Colour complement( Colour colour1, Colour colour2 ) {
    return colour1 == colour2 ? colour1 : (Colour)(3 - colour1 - colour2);
} // complement

struct MeetMsg : public uActor::Message {
    Colour colour;
    MeetMsg( Colour colour ) : Message( uActor::Delete ), colour( colour ) {}
};
struct ChangeMsg : public uActor::Message {
    Colour colour;
    ChangeMsg( Colour colour ) : Message( uActor::Delete ), colour( colour ) {}
};
struct MeetingCountMsg : public uActor::Message {
    int count;
    MeetingCountMsg( int count ) : Message( uActor::Delete ), count( count ) {}
};
struct ExitMsg : public uActor::Message {} exitMsg;


_Actor Mall;						// forward declaration

_Actor Chameneos {
    Mall *mall;
    Colour colour;
    int meetings = 0;

    void preStart();
    Allocation receive( Message &msg );
  public:
    int cid;
    Chameneos( Mall *mall, Colour colour, int cid ) : mall( mall ), colour( colour ), cid( cid ) {}
}; // Chameneos

_Actor Mall {
    Chameneos *waitingChameneo = nullptr;
    int n, check, numChameneos, sumMeetings = 0, numFaded = 0;

    Allocation receive( Message &msg ) {
	Case( MeetMsg, msg ) {
	    if ( n > 0 ) {
		if ( waitingChameneo ) {
		    PRT( osacquire( cout ) << ((Chameneos *)(msg.sender))->cid << " arrived at mall and matched with " << waitingChameneo->cid << endl; )
		    n -= 1;
		    assert( ((void)"mall, MeetMsg waitingChameneo == nullptr", waitingChameneo) );
		    waitingChameneo->tell( *new MeetMsg( msg_t->colour ), msg.sender );
		    waitingChameneo = nullptr;
		} else {
		    waitingChameneo = dynamic_cast<Chameneos *>(msg.sender);
		    PRT( osacquire( cout ) << waitingChameneo->cid << " arrived at mall but no partner (waiting)" << endl; )
		    assert( ((void)"mall, MeetMsg not Chameneos", waitingChameneo) );
		} // if
	    } else {
		PRT( osacquire( cout ) << ((Chameneos *)(msg.sender))->cid << " exit" << endl; )
		msg.sender->tell( exitMsg, this );
	    } // if
	} else Case( MeetingCountMsg, msg ) {
	    numFaded += 1;
	    sumMeetings += msg_t->count;
	    if ( numFaded == numChameneos ) {
		osacquire( cout ) << "meetings " << sumMeetings << " check " << check << endl;
		return Delete;
	    } // if
        } else {
	    assert( ((void)"Mall, unhandled message", false) );
        } // Case
	return Nodelete;				// reuse actor
    } // Mall::receive
  public:
    Mall( int n, int numChameneos ) : n( n ), check( n + n ), numChameneos( numChameneos ) {
	for ( int i = 0; i < numChameneos; i += 1 ) {
	    new Chameneos( this, (Colour)(i % 3), i );
	} // for
    } // Mall::Mall
}; // Mall


void Chameneos::preStart() {
    mall->tell( *new MeetMsg( colour ), this );
} // Chameneos::preStart

uActor::Allocation Chameneos::receive( Message &msg ) {
    Case( MeetMsg, msg ) {
	Colour prev = colour;
	colour = complement( colour, msg_t->colour);
	assert( ((void)"Chameneos, sender == nullptr", msg_t->sender) );
	assert( ((void)"Chameneos, sender == Mall", ! dynamic_cast<Mall *>(msg_t->sender)) );
	PRT( osacquire( cout ) << cid << " " << ColourNames[prev] << " meets " << ((Chameneos *)(msg.sender))->cid << " " << ColourNames[msg_t->colour]
	     << " and changes to " << ColourNames[colour] << endl; )
	meetings += 1;
	msg_t->sender->tell( *new ChangeMsg( colour ), this );
	mall->tell( *new MeetMsg( colour ), this );
    } else Case( ChangeMsg, msg ) {
	PRT( osacquire( cout ) << cid << " change from " << ColourNames[colour] << " to " << ColourNames[msg_t->colour] << endl; )
	colour = msg_t->colour;
	meetings += 1;
	mall->tell( *new MeetMsg( colour ), this );
    } else Case( ExitMsg, msg ) {
	msg.sender->tell( *new MeetingCountMsg( meetings ), this );
	return Delete;
    } else {
	assert( ((void)"Mall, unhandled message", false) );
    } // Case
    return Nodelete;				// reuse actor
} // Chameneos::receive


int main( int argc, char *argv[] ) {
    int nHost = 5, nCham = 5;				// default values

    try {
	switch ( argc ) {
	  case 3:
	    nCham = stoi( argv[1] );
	    if ( nCham < 1 ) throw 1;
	  case 2:
	    nHost = stoi( argv[2] );
	    if ( nHost < 1 ) throw 1;
	  case 1:					// use defaults
	    break;
	  default:
	    throw 1;
	} // switch
    } catch( ... ) {
	cout << "Usage: " << argv[0] << " [ nHost (> 0) [ nCham (> 0) ] ]" << endl;
	exit( 1 );
    } // try

    new Mall( nHost, nCham );
    uActor::stop();					// wait for all actors to terminate
} // main

// Local Variables: //
// compile-command: "u++-work -g -O2 -multi ActorChameneos.cc" //
// End: //
