#include <iostream>
using namespace std;
#include <cerrno>					// errno
#include <cstdlib>					// atoi, exit, abort
#include <cstring>					// string routines
#include <cassert>					// assert
#include <sys/socket.h>
#include <sys/param.h>					// howmany
#include <sys/un.h>
#include <fcntl.h>

#ifndef SUN_LEN
#define SUN_LEN(su) (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

const char EOD = '\377';

int main() {
    int sock, code, len;
    socklen_t saddrlen;
    struct sockaddr_un saddr;
    static fd_set rfds, wfds;				// implicitly zero filled
    char msg[256];
    unsigned int rcnt = 0, wcnt = 0;

    sock = socket( AF_UNIX, SOCK_DGRAM, 0 );		// create data-gram socket
    if ( sock < 0 ) {
	perror( "Error: server socket" );
	abort();
    } // if

    if ( fcntl( sock, F_SETFL, O_NONBLOCK ) < 0 ) {	// make socket non-blocking
	perror( "Error: server non-blocking" );
	abort();
    } // if

    memset( &saddr, '\0', sizeof(saddr) );
    saddr.sun_family = AF_UNIX;
    strcpy( saddr.sun_path, "sock" );
    saddrlen = SUN_LEN( &saddr );

    if ( bind( sock, (struct sockaddr *)&saddr, saddrlen ) == -1 ) {
	perror( "Error: server bind" );
	abort();
    } // if

    for ( ;; ) {
	saddrlen = sizeof( saddr );			// must be initialized to "from" buffer size
	len = recvfrom( sock, msg, sizeof(msg), 0, (struct sockaddr *)&saddr, &saddrlen );
	if ( len == -1 ) {
	    if ( errno != EWOULDBLOCK ) {
		perror( "Error: server recvfrom" );
		abort();
	    } // if

	    timespec timeout = { 10, 0 };		// reset as select may update to indicate time left
	    FD_SET( sock, &rfds );
	    code = pselect( sock+1, &rfds, NULL, NULL, &timeout, NULL );
	    if ( code < 0 ) abort();
      if ( code == 0 ) break;				// timeout ?
	    // attempt receive again
	} else {
	    rcnt += len;
	    // sent data back to client
	    for ( ;; ) {
		int check = len;
		len = sendto( sock, msg, len, 0, (struct sockaddr *)&saddr, saddrlen );
		assert( check == len );
	      if ( len != -1 ) break;
		if ( errno != EWOULDBLOCK ) {
		    perror( "Error: server sendto" );
		    abort();
		} // if

		FD_SET( sock, &wfds );
		code = pselect( sock+1, NULL, &wfds, NULL, NULL, NULL );
		if ( code <= 0 ) abort();
		// attempt sendto again
	    } // for
	    wcnt += len;
	} // if
    } // for

    close( sock );
    if ( unlink( "sock" ) == -1 ) {
	perror( "Error: server unlink" );
	abort();
    } // if

    cerr << "server ending, rcnt:" << rcnt << " wcnt:" << wcnt << endl;
} // main

// Local Variables: //
// compile-command: "g++ -g serverUNIXDGRAM.cc -o server2" //
// End: //
