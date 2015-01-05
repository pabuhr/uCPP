#include <iostream>
using namespace std;
#include <cerrno>					// errno
#include <cstdlib>					// atoi, exit, abort
#include <cstring>					// string routines
#include <sys/socket.h>
#include <sys/un.h>
#ifdef __FreeBSD__
#include <sys/stat.h>
#endif // __FreeBSD__
#include <fcntl.h>

#ifndef SUN_LEN
#define SUN_LEN(su) (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

const char EOD = '\377';
const unsigned int BufSize = 256;
char rbuf[BufSize];
unsigned int rcnt, wcnt;


int main() {
    static fd_set rfds, wfds;				// zero filled
    int sock;
    socklen_t saddrlen;
    struct sockaddr_un server;
    int rlen, wlen;

    sock = socket( AF_UNIX, SOCK_STREAM, 0 );		// create stream socket
    if ( sock < 0 ) {
	perror( "Error: client socket" );
	abort();
    } // if

    if ( fcntl( sock, F_SETFL, O_NONBLOCK ) < 0 ) {	// make socket non-blocking
	perror( "Error: client non-blocking" );
	abort();
    } // if

    memset( &server, '\0', sizeof(server) );		// construct server address
    server.sun_family = AF_UNIX;
    strcpy( server.sun_path, "sock" );
    saddrlen = SUN_LEN( &server );

    for ( ;; ) {
      if ( connect( sock, (struct sockaddr *)&server, saddrlen ) != -1 ) break;
	if ( errno == EWOULDBLOCK ) {			// try again when ready
	    FD_SET( sock, &rfds );
	    if ( pselect( sock+1, &rfds, NULL, NULL, NULL, NULL ) == -1 ) {
		perror( "Error: client connect1" );
		abort();
	    } // if
	} else if ( errno == EINPROGRESS ) {		// wait for a non-blocking connect to complete
	    FD_SET( sock, &wfds );
	    if ( pselect( sock+1, NULL, &wfds, NULL, NULL, NULL ) == -1 ) {
		perror( "Error: client connect2" );
		abort();
	    } // if
	    // check if connection completed
	    int retcode;
	    socklen_t retcodeLen = sizeof(retcode);
	    if ( getsockopt( sock, SOL_SOCKET, SO_ERROR, &retcode, &retcodeLen ) == -1 ) {
		perror( "Error: client connect3" );
		abort();
	    } // if
	    // Do not attempt connect again! It's done.
	    break;
	} else {					// failed
	    perror( "Error: client connect4" );
	    abort();
	} // if
    } //  for

    char fileName[FILENAME_MAX];
    fscanf( stdin, "%s", &fileName[0] );
    //fprintf( stderr, "client fileName:%s\n", fileName );
    struct stat info;
    if ( stat( fileName, &info ) == -1 ) {
	perror( "Error: client stat failure" );
	abort();
    } // if
    wcnt = info.st_size;

    for ( ;; ) {
	wlen = write( sock, fileName, strlen( fileName ) );
      if ( wlen != -1 ) break;
	if ( errno != EWOULDBLOCK
#ifdef __FreeBSD__
	     && errno != ENOBUFS
#endif // __FreeBSD__
	    ) {						// real problem ?
	    perror( "Error: client write" );
	    abort();
	} // if
	FD_SET( sock, &wfds );
	if ( pselect( sock+1, NULL, &wfds, NULL, NULL, NULL ) == -1 ) {
	    perror( "Error: client write1" );
	    abort();
	} // if
    } // for

    for ( ;; ) {
	for ( ;; ) {
	    rlen = read( sock, rbuf, sizeof(rbuf) );
	  if ( rlen != -1 ) break;
	    if ( errno != EWOULDBLOCK ) {
		perror( "Error: client read" );
		abort();
	    } // if
	    FD_SET( sock, &rfds );			// wait for connection
	    if ( pselect( sock+1, &rfds, NULL, NULL, NULL, NULL ) == -1 ) {
		perror( "Error: client read1" );
		abort();
	    } // if
	} // for
      if ( rlen == 0 ) break;
	fwrite( rbuf, rlen, 1, stdout );
	rcnt += rlen;
    } // for

    close( sock );

    if ( wcnt != rcnt ) {
	cerr << "Error: client not all data transfered, wcnt:" << wcnt << " rcnt:" << rcnt << endl;
    } // if
} // main

// Local Variables: //
// compile-command: "g++ -g clientUNIXSTREAMSendfile.cc -o Client" //
// End: //
