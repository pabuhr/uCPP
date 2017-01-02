//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2016
// 
// ActorFork.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Dec 19 08:22:05 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Dec 27 11:24:56 2016
// Update Count     : 8
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

#ifdef NOOUTPUT						// disable printing for experiments
#define PRT( stmt )
#else
#define PRT( stmt ) stmt
#endif // NOOUTPUT

unsigned int uDefaultActorThreads() { return 1; }
unsigned int uDefaultActorProcessors() { return 0; }

struct StartMsg : public uActor::Message {} startMsg;

int MaxLevel = 3;					// default value
unsigned int createCnt = 0;				// count created actors

_Actor Fork {
    unsigned int currLevel;
    uActor * left, * right;

    Allocation receive( Message &msg ) {
	Case( StartMsg, msg ) {
	    PRT( osacquire( cout ) << this << " create currLevel " << currLevel << endl; )
	    if ( currLevel < (unsigned int)MaxLevel ) {
		*(left = new Fork( currLevel + 1 )) | startMsg;
		*(right = new Fork( currLevel + 1 )) | startMsg;
	    } // if
	} // Case
	return Delete;
    } // Fork::receive
  public:
    Fork( unsigned int currLevel ) : currLevel( currLevel ) {
	uFetchAdd( createCnt, 1 );
    } // Fork::Fork
}; // Fork

void uMain::main() {
    try {
	switch ( argc ) {
	  case 2:
	    MaxLevel = stoi( argv[1] );
	    if ( MaxLevel < 1 ) throw 1;
	  case 1:					// use defaults
	    break;
	  default:
	    throw 1;
	} // switch
    } catch( ... ) {
	cout << "Usage: " << argv[0] << " [ maximum level (> 0) ]" << endl;
	exit( 1 );
    } // try

    PRT( cout << "MaxLevel " << MaxLevel << endl; )
    MaxLevel -= 1;					// decrement to handle created leaves
    Fork *root = new Fork( 0 );
    *root | startMsg;
    uActor::stop();
    cout << createCnt << " actors created" << endl;
} // uMain::main

// Local Variables: //
// compile-command: "u++-work -Wall -g -O2 -multi ActorFork.cc" //
// End: //
