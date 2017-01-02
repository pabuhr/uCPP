//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Roy Krischer 2002
// 
// EHM4.cc -- 
// 
// Author           : Roy Krischer
// Created On       : Tue Mar 26 23:01:30 2002
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Dec 19 08:44:20 2016
// Update Count     : 148
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

#include <uBarrier.h>
#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

#define NTASK 5
#define ROUNDS 10000


_Monitor atomicCnt {
    int c;
  public:
    atomicCnt( int c = -1 ) : c(c) {}

    int inc() {
		c += 1;
		return c;
    } // inc
}; // atomicCnt


_Task worker {
    int id, round;

	void main();
  public:
    worker ( int id ) : id(id), round(ROUNDS) {
		osacquire( cout ) << "task " << this << " creation" << endl;
	} 
    ~worker() {
		osacquire( cout ) << "task " << this << " destruction" << endl;
    }
}; // worker


atomicCnt cnt;											// atomic counter
int array[NTASK*((NTASK-1)*ROUNDS+1)] = {0};			// check for duplicate handling
int handled[NTASK] = {0};								// count exceptions handled per task
uBarrier b( NTASK + 1 );								// control start and finish of main/worker tasks
worker *f[NTASK];										// shared resource controlled by barrier

_Event Rev {
  public:
    int ticket;
    Rev( const char *msg, int ticket ) : ticket( ticket ) { setMsg( msg ); }
};

void worker::main() {
    b.block();											// wait for all tasks to start
	osacquire( cout ) << "task " << this << " starting" << endl;
	yield( NTASK );

	try {
		_Enable {
			_Resume Rev( "self", cnt.inc() ) _At *this; // initial resume at myself
			for ( int n = 0; n < ROUNDS / 2; n += 1 ) {	// generate other 1/2 of the exceptions
				yield();								// allow delivery of concurrent resumes
				for ( int i = 0; i < NTASK; i += 1 ) {	// send exceptions to other tasks
					if ( i != id ) {					// except myself
						_Resume Rev( "other", cnt.inc() ) _At *f[i];
					} // if
				} // for
			} // for
		} // _Enable
	} _CatchResume ( Rev &r ) {
		handled[id] += 1;								// count exceptions handled by each task
		//osacquire( cerr ) << "handler, exception id: " << e.ticket << endl;
		assert( r.ticket < NTASK*((NTASK-1)*ROUNDS+1) ); // subscript error ?
		array[r.ticket] += 1;
		if ( array[r.ticket] > 1 ) uAbort( "same exception handled twice");
		if ( round != 0 ) {								// only a subset of exceptions raise more
			round -= 1;
			if ( round % 2 == 0 ) {						// generate 1/2 of the exceptions
				for ( int i = 0; i < NTASK; i += 1 ) {	// send exceptions to other tasks
					if ( i != id ) {					// except myself
						_Resume Rev( "other", cnt.inc() ) _At *f[i];
					} // if
				} // for
			} // if
		} // if
	} // try

    b.block();											// wait for all tasks to finish
	osacquire( cout ) << "task " << this << " finishing" << endl;
} // worker::main


void uMain::main () {
    uProcessor processors[4] __attribute__(( unused ));	// more than one processor

    for ( int i = 0; i < NTASK; i += 1 ) {
		f[i] = new worker( i );
    } // for
    b.block();											// wait for all tasks to start

    b.block();											// wait for all tasks to finish
	int total = 0;
    for ( int i = 0; i < NTASK; i += 1 ) {
		delete f[i];
		total += handled[i];							// sum exceptions handled by each task
    } // for
	osacquire( cout ) << "cnt:" << cnt.inc() << "  handled:" << total << endl;
} // uMain::main


// Local Variables: //
// tab-width: 4 //
// End: //
