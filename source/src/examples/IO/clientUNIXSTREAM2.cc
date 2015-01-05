#include <iostream>
using namespace std;
#include <cerrno>					// errno
#include <cstdlib>					// atoi, exit, abort
#include <cstring>					// string routines
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#ifndef SUN_LEN
#define SUN_LEN(su) (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

const char EOD = '\377';
const char EOT = '\376';
const unsigned int BufSize = 256;
// buffer must be retained between calls due to blocking write (see parameter "state")
char wbuf[BufSize+1];					// +1 for '\0'
bool wblk;
unsigned int rcnt, wcnt, written;


bool input( char *inmsg, int size ) {
    static bool eod = false;

  if ( eod ) return true;
    cin.get( inmsg, size, '\0' );			// leave room for string terminator
    if ( inmsg[0] == '\0' ) {				// eof ?
	inmsg[0] = EOD;					// create EOD message for server
	inmsg[1] = '\0';
	eod = true;					// remember EOD is sent
    } // if
    return false;
} // input


bool reader( int sock ) {
    static char buf[BufSize];

    for ( ;; ) {
	int rlen = read( sock, &buf, BufSize );		// read data from socket
	if ( rlen == -1 ) {				// problem ?
	    if ( errno != EWOULDBLOCK ) {		// real problem ?
		perror( "Error: client read" );
		abort();
	    } // if
	    return false;				// indicate blocking
	} // if
	//cerr << "reader rlen:" << rlen << endl;
	if ( rlen == 0 ) {				// EOF should not occur
	    cerr << "Error: client EOF ecountered without EOD" << endl;
	    abort();
	} // if
	rcnt += rlen;					// total amount read
	if ( buf[rlen - 1] == EOD ) {			// EOD marks all read
	    cout.write( buf, rlen - 1 );		// write out data
	    //cerr << "reader seen EOD, setup EOT" << endl;
	    wbuf[0] = EOT;
	    wbuf[1] = 0;
	    return true;				// indicate EOD
	} // if
	cout.write( buf, rlen );			// write data
    } // for
} // reader


enum Wstatus { Blocked, Eod, Eot };

Wstatus writer( int sock ) {
    static bool eod = false;

  if ( eod && wbuf[0] != EOT ) return Eod;
    for ( ;; ) {
	if ( ! wblk ) {					// not write blocked ?
	    input( wbuf, BufSize+1 );			// read data if no previous blocking
	    written = 0;				// starting new write
	} // if
	for ( ;; ) {					// ensure all data is written
	    char *buf = wbuf + written;			// location to continue from in buffer
	    unsigned int len = strlen(wbuf) - written;	// amount left to write
	    int wlen = write( sock, buf, len );		// write data to socket
	    if ( wlen == -1 ) {				// problem ?
		if ( errno != EWOULDBLOCK ) {		// real problem ?
		    perror( "Error: client write" );
		    abort();
		} // if
		wblk = true;
		return Blocked;				// indicate blocking
	    } // if
	    wblk = false;
	    //cerr << "writer wlen:" << wlen << endl;
	    wcnt += wlen;				// total amount written
	    if ( wbuf[wlen - 1] == EOD ) {		// EOD marks all written
		eod = true;
		//cerr << "writer EOD" << endl;
		return Eod;				// indicate EOD
	    } // if
	    if ( wbuf[wlen - 1] == EOT ) {		// EOT marks all written
		wcnt -= 1;				// doesn't apply, remove
		//cerr << "writer EOT" << endl;
		return Eot;				// indicate EOT
	    } // if
	    written += wlen;				// current amount written
	  if ( written == strlen(wbuf) ) break;		// transferred across all writes
	} // for
    } // for
} // writer


void manager( int sock ) {
    static fd_set mrfds, rfds, mwfds, wfds;		// zero filled
    bool weod = false;

    for ( ;; ) {
	//cerr << "manager1  weod:" << weod << " mwfds:" << mwfds.fds_bits[0] << " mrfds:" << mrfds.fds_bits[0] << endl;
	if ( ! FD_ISSET( sock, &mwfds ) ) {		// not waiting ?
	    Wstatus wstatus = writer( sock );
	    if ( wstatus == Eot ) break;		// EOT sent ?
	    if ( wstatus == Eod ) {
		weod = true;
	    } else {
		FD_SET( sock, &mwfds );			// blocking, set mask
	    } // if
	} // if
	if ( ! FD_ISSET( sock, &mrfds ) ) {		// not waiting ?
	    if ( reader( sock ) ) {			// EOD read ?
	    } else {
		FD_SET( sock, &mrfds );			// otherwise blocking, set mask
	    } // if
	} // if
	// both blocking or writer finished and reader blocking ?
	if ( ( FD_ISSET( sock, &mwfds ) || weod ) && FD_ISSET( sock, &mrfds ) ) {
	    //cerr << "manager2  weod:" << weod << " mwfds:" << mwfds.fds_bits[0] << " mrfds:" << mrfds.fds_bits[0] << endl;
	    rfds = mrfds;				// copy for destructive usage
	    wfds = mwfds;
	    int nfs = pselect( sock+1, &rfds, &wfds, NULL, NULL, NULL ); // wait for one or the other
	    if ( nfs <= 0 ) {
		perror( "Error: pselect" );
		abort();
	    } // if

	    //cerr << "manager3  nfds:" << nfs << " wfds:" << wfds.fds_bits[0] << " rfds:" << rfds.fds_bits[0] << endl;
	    if ( FD_ISSET( sock, &mwfds ) && FD_ISSET( sock, &wfds ) ) { // waiting for data and data ?
		FD_CLR( sock, &mwfds );			// clear blocking
	    } // if

	    if ( FD_ISSET( sock, &mrfds ) && FD_ISSET( sock, &rfds ) ) { // waiting for data and data ?
		FD_CLR( sock, &mrfds );			// clear blocking
	    } // if
	} // if
    } // for
} // manager


int main() {
    static fd_set rfds, wfds;				// zero filled
    int sock;
    socklen_t saddrlen;
    struct sockaddr_un server;

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

    manager( sock );

    close( sock );

    if ( wcnt != rcnt ) {
	cerr << "Error: client not all data transfered, wcnt:" << wcnt << " rcnt:" << rcnt << endl;
    } // if
} // main

// Local Variables: //
// compile-command: "g++ -g clientUNIXSTREAM2.cc -o Client" //
// End: //
