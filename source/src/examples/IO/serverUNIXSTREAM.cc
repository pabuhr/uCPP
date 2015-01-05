#include <iostream>
using namespace std;
#include <cerrno>					// errno
#include <cstdlib>					// atoi, exit, abort
#include <cstring>					// string routines
#include <cassert>					// assert
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#ifndef SUN_LEN
#define SUN_LEN(su) (sizeof(*(su)) - sizeof((su)->sun_path) + strlen((su)->sun_path))
#endif

const char EOD = '\377';
const unsigned int Simult = 5;
// state for each connection (need coroutines)
const unsigned int BufSize = 256;
char bufs[Simult][BufSize];
int fds[Simult];					// maximum of N simultaneous connections (0 => not used)
int rlen[Simult], written[Simult];
bool wblk[Simult];					// set to false (0)
// select masks
fd_set mrfds, rfds, mwfds, wfds;			// zero filled
// counters
unsigned int rcnt, wcnt, accepts = 1, waiting, clients, blocked, selected;
unsigned int NoOfClients;				// assume a total of N clients in T sets of Simult clients


void accepter( int sock ) {
    int fd = accept( sock, NULL, NULL );
    if ( fd == -1 ) {					// problem ?
	if ( errno != EWOULDBLOCK ) {			// real problem ?
	    perror( "Error: server accept" );
	    abort();
	} // if
	blocked += 1;
	waiting += 1;
	FD_SET( sock, &mrfds );				// blocking, set mask
	return;
    } // if
    // On some UNIX systems the file descriptor created by accept inherits the non-blocking characteristic from the base
    // socket; on other system this does not seem to occur, so explicitly set the file descriptor to non-blocking.
    if ( fcntl( fd, F_SETFL, O_NONBLOCK ) < 0 ) {	// make fd non-blocking
	perror( "Error: server accept non-blocking" );
	abort();
    } // if
    for ( unsigned int i = 0; i < Simult; i += 1 ) {	// insert into an empty location
	if ( fds[i] == 0 ) {
	    accepts += 1;
	    //cerr << "accept fd:" << fd << " posn:" << i << endl;
	    fds[i] = fd;
	    return;					// indicate accept
	} // if
    } // for
    assert( ((void)"too many clients", false) );
} // accepter


void rdwr( int i ) {
    int wlen;

    //unsigned int times = rand() % 2 == 0 ? 20 : 200;
    unsigned int times = 1000000;
    for ( unsigned int j = 0; j < times; j += 1 ) {
	if ( ! wblk[i] ) {				// not write blocked ?
	    rlen[i] = read( fds[i], bufs[i], BufSize );	// read data from socket
	    if ( rlen[i] == -1 ) {			// problem ?
		if ( errno != EWOULDBLOCK ) {		// real problem ?
		    perror( "Error: server read" );
		    abort();
		} // if
		blocked += 1;
		waiting += 1;
		FD_SET( fds[i], &mrfds );		// blocking, set mask
		return;
	    } // if
	    //cerr << "reader(" << fds[i] << ") rlen:" << rlen[i] << endl;
	    if ( rlen[i] == 0 ) {			// EOF should not occur
		cerr << "Error: server EOF ecountered without EOD" << endl;
		abort();
	    } // if
	    rcnt += rlen[i];				// total amount read
	} // if

	if ( ! wblk[i] ) {				// not write blocked ?
	    written[i] = 0;				// starting new write
	} // if
	for ( ;; ) {					// ensure all data is written
	    char *buf = bufs[i] + written[i];		// location to continue from in buffer
	    unsigned int len = rlen[i] - written[i];	// amount left to write
	    wlen = write( fds[i], buf, len );		// write data to socket
	    if ( wlen == -1 ) {				// problem ?
		if ( errno != EWOULDBLOCK ) {		// real problem ?
		    //cerr << "write(" << fds[i] << "," << &bufs[i] << "," << rlen[i] << ")" << endl;
		    perror( "Error: server write" );
		    abort();
		} // if
		wblk[i] = true;				// don't read on restart
		blocked += 1;
		waiting += 1;
		FD_SET( fds[i], &mwfds );		// blocking, set mask
		return;
	    } // if
	    wblk[i] = false;				// did not block
	    //cerr << "writer(" << fds[i] << ") wlen:" << wlen << endl;
	    wcnt += wlen;				// total amount written
	    if ( bufs[i][wlen - 1] == EOD ) {		// EOD marks all written
		close( fds[i] );			// close connection
		fds[i] = 0;				// mark for reuse
		accepts -= 1;
		clients += 1;				// client finished
		//cerr << "clients:" << clients << " " << NoOfClients << endl;
		return;
	    } // if
	    written[i] += wlen;				// current amount written
	  if ( written[i] == rlen[i] ) break;		// transferred across all writes
	} // for
    } // for
} // writer


void manager( int sock ) {
    unsigned int i;

    for ( ;; ) {
	if ( ! FD_ISSET( sock, &mrfds ) ) { 		// not waiting ?
	    accepter( sock );
	} // if

	for ( i = 0; i < Simult; i += 1 ) {		// try I/O on all active clients
	  if ( fds[i] == 0 ) continue;			// skip non-connections
	    //cerr << "manager1  accept:" << i << hex << " wfds:" << wfds.fds_bits[0] << " rfds:" << mrfds.fds_bits[0] << dec << endl;
	    if ( ! FD_ISSET( fds[i], &mrfds ) && ! FD_ISSET( fds[i], &mwfds ) ) { // not waiting ?
		rdwr( i );
	    } // if
	} // for

      if ( clients == NoOfClients ) break;		// all clients serviced ?

	//cerr << "manager1.1  accepts:" << accepts << " waiting:" << waiting << endl;
	if ( accepts == waiting ) {			// all blocked?
	    //cerr << "manager2  nfs:" << Simult + 4 << hex << " wfds:" << mwfds.fds_bits[0] << " rfds:" << mrfds.fds_bits[0] << dec << endl;
	    int nfs;
	    rfds = mrfds;				// copy for destructive usage
	    wfds = mwfds;
	    selected += 1;
	    if ( (nfs = pselect( Simult + 4, &rfds, &wfds, NULL, NULL, NULL )) <= 0 ) { // wait for event
		perror( "Error: pselect" );
		abort();
	    } // if
	    //cerr << "manager3  nfs:" << nfs << hex << " wfds:" << wfds.fds_bits[0] << " rfds:" << rfds.fds_bits[0] << dec << endl;

	    // handle accepts
	    if ( FD_ISSET( sock, &mrfds ) && FD_ISSET( sock, &rfds ) ) { // waiting for data and data ?
		//cerr << "manager4  wfds:" << mwfds.fds_bits[0] << " rfds:" << mrfds.fds_bits[0] << endl;
		waiting -= 1;
		FD_CLR( sock, &mrfds );			// clear blocking
	    } // if
	    // handle clients
	    for ( i = 0; i < Simult; i += 1 ) {		// check all the bits, ignore 0,1,2, socket 3, fds 4-14
		//cerr << "manager5  wfds:" << hex << mwfds.fds_bits[0] << " rfds:" << mrfds.fds_bits[0] << dec << endl;
	      if ( fds[i] == 0 ) continue;
		if ( FD_ISSET( fds[i], &mwfds ) && FD_ISSET( fds[i], &wfds ) ) { // waiting for data and data ?
		    waiting -= 1;
		    FD_CLR( fds[i], &mwfds );		// clear blocking
		} // if

		if ( FD_ISSET( fds[i], &mrfds ) && FD_ISSET( fds[i], &rfds ) ) { // waiting for data and data ?
		    waiting -= 1;
		    FD_CLR( fds[i], &mrfds );		// clear blocking
		} // if
		//cerr << "manager6  wfds:" << hex << mwfds.fds_bits[0] << " rfds:" << mrfds.fds_bits[0] << dec << endl;
	    } // for
	} // if
    } // for
} // manager


int main( int argc, char *argv[] ) {
    unsigned int times;

    switch ( argc ) {
      case 3:
	times = atoi( argv[1] );
	break;
      default:
	cerr << "Usage: " << argv[0] << " times socket-name" << endl;
	exit( EXIT_FAILURE );
    } // switch

    NoOfClients = times * Simult;			// assume a total of N clients in T sets of 5 clients

    int sock;
    socklen_t saddrlen;
    struct sockaddr_un server;

    sock = socket( AF_UNIX, SOCK_STREAM, 0 );		// create stream socket
    if ( sock < 0 ) {
	perror( "Error: server socket" );
	abort();
    } // if

    if ( fcntl( sock, F_SETFL, O_NONBLOCK ) < 0 ) {	// make socket non-blocking
	perror( "Error: server non-blocking" );
	abort();
    } // if

    memset( &server, '\0', sizeof(server) );
    server.sun_family = AF_UNIX;
    strcpy( server.sun_path, argv[2] );
    saddrlen = SUN_LEN( &server );

    if ( bind( sock, (struct sockaddr *)&server, saddrlen ) == -1 ) {
	perror( "Error: server bind" );
	abort();
    } // if

    if ( listen( sock, 10 ) == -1 ) {
	perror( "Error: server listen" );
	abort();
    } // if

    manager( sock );

    close( sock );

    if ( wcnt != rcnt ) {
	cerr << "Error: server not all data transfered, wcnt:" << wcnt << " rcnt:" << rcnt << endl;
    } // if
    cout << "server ending, blocked:" << blocked << " selected:" << selected << endl;
} // main

// Local Variables: //
// compile-command: "g++ -g serverUNIXSTREAM.cc -o Server" //
// End: //
