#include <iostream>
using namespace std;
#include <cerrno>					// errno
#include <cassert>					// assert
#include <cstdlib>					// atoi, exit, abort
#include <cstring>					// string routines
#include <sys/socket.h>
#include <sys/param.h>					// MAXHOSTNAMELEN
#include <netdb.h>					// gethostbyname
#include <fcntl.h>
#ifdef __FreeBSD__
#include <netinet/in.h>
#endif // __FreeBSD__

int main( int argc, char *argv[] ) {
    int sock, code, len;
    socklen_t saddrlen;
    struct sockaddr_in saddr;
    unsigned short port;
    char name[MAXHOSTNAMELEN+1];
    struct hostent *hp;
    static fd_set rfds, wfds;				// implicitly zero filled
    char msg[256];
    unsigned int rcnt = 0, wcnt = 0;

    sock = socket( AF_INET, SOCK_DGRAM, 0 );		// create data-gram socket
    if ( sock < 0 ) {
	perror( "Error: server socket" );
	abort();
    } // if

    if ( fcntl( sock, F_SETFL, O_NONBLOCK ) < 0 ) {	// make socket non-blocking
	perror( "Error: server non-blocking" );
	abort();
    } // if

    if ( gethostname( name, sizeof(name) ) == -1 ) {	// get host name for this system
	perror( "Error: server gethostname" );
	exit( EXIT_FAILURE );
    } // if

    hp = gethostbyname( name );				// get internet address from internet name
    if ( hp == NULL ) {
	perror( "Error: server gethostbyname" );
	exit( EXIT_FAILURE );
    } // if

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons( 0 );			// let host select a free port number
    memcpy( &(saddr.sin_addr), hp->h_addr, hp->h_length );
    memset( &(saddr.sin_zero), '\0', sizeof(saddr.sin_zero) );

    if ( bind( sock, (struct sockaddr *)&saddr, sizeof(saddr) ) == -1 ) {
	perror( "Error: server bind" );
	abort();
    } // if

    saddrlen = sizeof(saddr);				// get host selected port number
    if ( getsockname( sock, (struct sockaddr *)&saddr, &saddrlen ) == -1 ) {
	perror( "Error: getsockname" );
	exit( EXIT_FAILURE );
    } // if
    port = ntohs( saddr.sin_port );

    cout << port;					// print out free port for clients
    cout.flush();					// print NOW for clients

    for ( ;; ) {
	saddrlen = sizeof( saddr );
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
		if ( errno != EWOULDBLOCK
#ifdef __FreeBSD__
		     && errno != ENOBUFS
#endif // __FreeBSD__
		    ) {					// real problem ?
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

    cerr << "server ending, rcnt:" << rcnt << " wcnt:" << wcnt << endl;
} // main

// Local Variables: //
// compile-command: "g++ -g serverINETDGRAM.cc -o Server" //
// End: //
