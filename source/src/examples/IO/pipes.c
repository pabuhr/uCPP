#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>					// exit
#include <unistd.h>					// pipe
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>					// EAGAIN
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifndef timersub
#define timersub(a, b, result)				\
  do {							\
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;	\
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;	\
    if ((result)->tv_usec < 0) {			\
      --(result)->tv_sec;				\
      (result)->tv_usec += 1000000;			\
    }							\
  } while (0)
#endif // timersub

#define PIPE_NUM 510
#define PIPE_FDS (PIPE_NUM * 2)
static int pipe_fds[PIPE_FDS];
static int maxfd = 0;

static void open_pipes() {
    int i;

    for ( i = 0; i < PIPE_FDS; i += 2 ) {
	if ( pipe( &pipe_fds[i] ) < 0 ) {
	    perror( "pipe" );
	    exit( 1 );
	} // if
	if ( pipe_fds[i] > maxfd ) maxfd = pipe_fds[i];
	if ( pipe_fds[i+1] > maxfd ) maxfd = pipe_fds[i+1];

	if ( fcntl( pipe_fds[i], F_SETFL, O_NONBLOCK ) < 0 ) {
	    perror( "fcntl" );
	    exit( 1 );
	} // if
	if ( fcntl( pipe_fds[i+1], F_SETFL, O_NONBLOCK ) < 0 ) {
	    perror( "fcntl" );
	    exit( 1 );
	} // if
/*
*/
    } // for
} // open_pipes

static void clean_up() {
    int i;
    for ( i = 0; (i < PIPE_FDS) && (pipe_fds[i] > 0 ); i += 1 )
	close( pipe_fds[i] );
} // clean_up

static void sig_handler( int signum ) {
    (void)signum;
    exit(0);
}

static int select_calls;
static int select_fds;
static int do_reader_bytes;

static void do_reader_work( int fd ) {
    static char buf[8192];
    int nread;
    int i, j;

    // Read from the pipe.
    if ( (nread = read( fd, buf, 8192 )) < 0 ) {
	perror( "read" );
	abort();
    } // if

    for ( i = 0; i < nread; i += 1 ) {		// check transfer
	if ( buf[i] != 'a' ) abort();
    } // for

    // Do some arbitrary work on the buffer.
    // Change the outer loop limit to control the amount of work done per read call.
    for ( j = 0; j < 1; j += 1 ) {
	for ( i = 0; i < nread - 3; i += 1 ) {
	    buf[i + 1] ^= buf[i];
	    buf[i + 2] |= buf[i + 1];
	    buf[i + 3] &= buf[i + 2];
	} // for
    } // for

    do_reader_bytes += nread;
} // do_reader_work


static void *read_pipes( void *arg ) {
    fd_set rfds;
    int i;
    struct timeval stv, etv, dtv;
    double do_read_mbps;
    int nfds;
	
    (void)arg;

    printf( "Starting reader thread %ld\n", (size_t)arg );

    // Prepare an initial read descriptor set.
    FD_ZERO( &rfds );
    for (i = 0; i < PIPE_FDS; i+= 2) {
	FD_SET( pipe_fds[i], &rfds );
    } // for

    gettimeofday( &stv, NULL );
	
    // Run the select loop.
    struct timeval t = { 5, 0 };
    while ( (nfds = select( maxfd + 1, &rfds, NULL, NULL, &t )) > 0 ) {
	for ( i = 0; i < PIPE_FDS; i += 2 ) {
	    if ( FD_ISSET( pipe_fds[i], &rfds ) )
		do_reader_work( pipe_fds[i] );
	    else
		FD_SET( pipe_fds[i], &rfds );
	} // for

	// Statistics.
	select_calls += 1;
	select_fds += nfds;
	gettimeofday( &etv, NULL );
	timersub( &etv, &stv, &dtv );

	if ( dtv.tv_sec >= 1 ) {
	    do_read_mbps = (double)do_reader_bytes / (double)((dtv.tv_sec * 1000000) + dtv.tv_usec);
	    printf( "Time: %ld.%06ld Calls: %d FDs/Call: %d TPut: %.2f MBps\n",
		   dtv.tv_sec, dtv.tv_usec, select_calls, select_fds / select_calls, do_read_mbps );
	    select_calls = 0;
	    select_fds = 0;
	    do_reader_bytes = 0.;
	    stv = etv;
	} // if
    } // while
	
    printf( "Ending reader thread %ld\n", (size_t)arg );
	
    return NULL;
} // read_pipes


static char buffer[8192];

#define NOWRITERS 3
volatile bool stopWriters = false;

static void *write_pipes( void *arg ) {
    int pipe, nfds, eagain = 0, times;
    fd_set wfds;
	
    printf( "Starting writer thread %ld\n", (size_t)arg );

    FD_ZERO( &wfds );
    pipe = ((random() % PIPE_NUM) * 2) + 1;

    for ( ; ! stopWriters; ) {
	// Write a page to a pipe.
	while ( write( pipe_fds[pipe], buffer, 8192 ) == -1 ) {
	    if ( errno != EAGAIN ) {
		perror( "write" );
		abort();
	    } // if
	    eagain += 1;
	    FD_SET( pipe_fds[pipe], &wfds );
	    nfds = select( pipe_fds[pipe] + 1, NULL, &wfds, NULL, NULL );
	    if ( nfds <= 0 ) {
		perror( "write select" );
		abort();
	    } // if
	    FD_CLR( pipe_fds[pipe], &wfds );
	} // while
	pipe += 2;
	if ( pipe >= PIPE_FDS ) pipe = 1;
    } // for

    printf( "Ending writer thread %ld  eagain %d\n", (size_t)arg, eagain );
	
    return NULL;
} // write_pipes


int main() {
    pthread_t reader_tid;
    pthread_t writer_tid[NOWRITERS];
    size_t i;
	
    // Close pipes upon exit.
    atexit( clean_up );
    signal( SIGINT, sig_handler );
	
    for ( i = 0; i < 8192; i += 1 ) {
	buffer[i] = 'a';
    } // for
		 
    // Create the pipes.
    open_pipes();

    // Create the reader thread.
    if ( pthread_create( &reader_tid, NULL, read_pipes, NULL ) < 0 ) {
	perror("pthread_create");
	exit(1);
    } // if

    // Create the writer threads.
    for ( i = 0; i < NOWRITERS; i += 1 ) {
	if ( pthread_create( &writer_tid[i], NULL, write_pipes, (void *)i ) < 0 ) {
	    perror( "pthread_create" );
	    exit( 1 );
	} // if
    } // for

    sleep( 30 );					// writers run for N seconds
    stopWriters = true;
	
    for ( i = 0; i < NOWRITERS; i += 1 )
 	pthread_join( writer_tid[i], NULL );
    pthread_join( reader_tid, NULL );

    return 0;
}

// Local Variables: //
// compile-command: "gcc -O2 -DFD_SETSIZE=65536 pipes.c -lpthread" //
// End: //
