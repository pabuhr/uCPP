//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard Bilson 2003
// 
// AbortExit.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Dec 19 10:33:58 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Sep 26 01:35:34 2012
// Update Count     : 16
// 

#include <uSemaphore.h>
#include <iostream>
#include <unistd.h>

using namespace std;

enum Mode { SPIN, EXIT, UABORT, EXPLODE, ABORT, ASSERT, RETURN, PTHREAD_RETURN } mode;

void *spinner( void * ) {
    for ( ;; );
} // spinner

_Task worker {
    void main() {
        switch( mode ) {
          case SPIN:
	    for ( ;; ) {}
	  case EXIT:
	    exit( EXIT );
	  case UABORT:
	    uAbort( "worker %d %s", UABORT, "text" );
	  case EXPLODE:
	    kill( getpid(), SIGKILL );
	    for ( ;; ) {}				// delay until signal delivered to some kernel thread
	  case ABORT:
	    abort();
	  case ASSERT:
	    assert( false );
	  default: ;
	} // switch
 	// CONTROL NEVER REACHES HERE!
    } // main
  public:
    worker( uCluster &c ) : uBaseTask( c ) {}
}; // worker

void uMain::main() {
    if ( argc <= 1 ) {
        cerr << "usage: " << argv[0] << " [0-7]" << endl;
	exit( EXIT_FAILURE );
    } // if
    mode = (Mode)atoi( argv[1] );

    switch (mode) {
      case RETURN:
	uRetCode = RETURN;
	return;
      case PTHREAD_RETURN:
	uRetCode = PTHREAD_RETURN;
	pthread_t pt;
	pthread_create( &pt, NULL, spinner, NULL );
	return;
      default:
	uCluster cluster;
	uProcessor processor( cluster );
	worker t( cluster );
	uSemaphore s( 0 );
	s.P();
    } // switch
} // uMain::main
