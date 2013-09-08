//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 2003
// 
// Allocation.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Oct  3 22:58:11 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Sep 26 01:36:29 2012
// Update Count     : 158
// 

#include <unistd.h>					// sbrk
#if defined( __linux__ )
#include <malloc.h>					// TEMPORARY: memalign missing from stdlib.h
#endif
#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;
#include <iomanip>
using std::setw;


_Task Worker {
    void main();
  public:
    Worker() : uBaseTask( 120 * 1000 ) {}		// larger stack
};


void Worker::main() {
    enum { NoOfAllocs = 5000 };
    char *locns[NoOfAllocs];
    int i;

    // check new/delete

    for ( int j = 0; j < 40; j += 1 ) {
	for ( i = 0; i < NoOfAllocs; i += 1 ) {
	    locns[i] = new char[i];
	    //cout << setw(6) << (void *)locns[i] << endl;
	    for ( int k = 0; k < i; k += 1 ) locns[i][k] = '\345';
	} // for
	//cout << (char *)sbrk(0) - start << " bytes" << endl;

	for ( i = 0; i < NoOfAllocs; i += 1 ) {
	    //cout << setw(6) << (void *)locns[i] << endl;
	    for ( int k = 0; k < i; k += 1 ) if ( locns[i][k] != '\345' ) uAbort( "new/delete corrupt storage1" );
	    delete [] locns[i];
	} // for
	//cout << (char *)sbrk(0) - start << " bytes" << endl;

	for ( i = 0; i < NoOfAllocs; i += 1 ) {
	    locns[i] = new char[i];
	    //cout << setw(6) << (void *)locns[i] << endl;
	    for ( int k = 0; k < i; k += 1 ) locns[i][k] = '\345';
	} // for
	for ( i = NoOfAllocs - 1; i >=0 ; i -= 1 ) {
	    //cout << setw(6) << (void *)locns[i] << endl;
	    for ( int k = 0; k < i; k += 1 ) if ( locns[i][k] != '\345' ) uAbort( "new/delete corrupt storage2" );
	    delete [] locns[i];
	} // for
    } // for

    // check malloc/free

    for ( i = 0; i < NoOfAllocs; i += 1 ) {
	size_t s = (i + 1) * 20;
	char *area = (char *)malloc( s );
	if ( area == NULL ) uAbort( "malloc/free out of memory" );
	area[0] = '\345'; area[s - 1] = '\345';		// fill first/last
	area[malloc_usable_size( area ) - 1] = '\345';	// fill ultimate byte
	free( area );
    } // for

    for ( i = 0; i < NoOfAllocs; i += 1 ) {
	size_t s = i + 1;				// +1 to make initialization simpler
	locns[i] = (char *)malloc( s );
	if ( locns[i] == NULL ) uAbort( "malloc/free out of memory" );
	locns[i][0] = '\345'; locns[i][s - 1] = '\345';	// fill first/last
	locns[i][malloc_usable_size( locns[i] ) - 1] = '\345'; // fill ultimate byte
    } // for
    for ( i = 0; i < NoOfAllocs; i += 1 ) {
	size_t s = i + 1;
	if ( locns[i][0] != '\345' || locns[i][s - 1] != '\345' ||
	     locns[i][malloc_usable_size( locns[i] ) - 1] != '\345' ) uAbort( "malloc/free corrupt storage" );
	free( locns[i] );
    } // for

    // check calloc/free

    for ( i = 0; i < NoOfAllocs; i += 1 ) {
	size_t s = i + 1;
	char *area = (char *)calloc( 1, s );
	if ( area == NULL ) uAbort( "calloc/free out of memory" );
	if ( area[0] != '\0' || area[s - 1] != '\0' ||
	     area[malloc_usable_size( area ) - 1] != '\0' ||
	     ! malloc_zero_fill( area ) ) uAbort( "calloc/free corrupt storage1" );
	area[0] = '\345'; area[s - 1] = '\345';		// fill first/last
	area[malloc_usable_size( area ) - 1] = '\345';	// fill ultimate byte
	free( area );
    } // for

    for ( i = 0; i < NoOfAllocs; i += 1 ) {
	size_t s = i + 1;
	locns[i] = (char *)calloc( 1, s );
	if ( locns[i] == NULL ) uAbort( "calloc/free out of memory" );
	if ( locns[i][0] != '\0' || locns[i][s - 1] != '\0' ||
	     locns[i][malloc_usable_size( locns[i] ) - 1] != '\0' ||
	     ! malloc_zero_fill( locns[i] ) ) uAbort( "calloc/free corrupt storage2" );
	locns[i][0] = '\345'; locns[i][s - 1] = '\345';	// fill first/last
	locns[i][malloc_usable_size( locns[i] ) - 1] = '\345'; // fill ultimate byte
    } // for
    for ( i = 0; i < NoOfAllocs; i += 1 ) {
	size_t s = i + 1;
	if ( locns[i][0] != '\345' || locns[i][s - 1] != '\345' ||
	     locns[i][malloc_usable_size( locns[i] ) - 1] != '\345' ) uAbort( "calloc/free corrupt storage3" );
	free( locns[i] );
    } // for

    // check memalign/free

    const size_t limit = 64 * 1024;			// check alignments up to here
    for ( size_t a = uAlign(); a <= limit; a += a ) {	// generate powers of 2
	//cout << setw(6) << alignments[a] << endl;
	for ( int s = 1; s < 64 * 1024; s += 1 ) {	// allocation of size 0 can return NULL
	    char *area = (char *)memalign( a, s );
	  if ( area == NULL ) uAbort( "memalign/free out of memory" );
	    //cout << setw(6) << i << " " << area << endl;
	    if ( (size_t)area % a != 0 || malloc_alignment( area ) != a ) { // check for initial alignment
		uAbort( "memalign/free bad alignment : memalign(%d,%d) = %p", (int)a, s, area );
	    } // if
	    area[0] = '\345'; area[s - 1] = '\345';	// fill first/last byte
	    area[malloc_usable_size( area ) - 1] = '\345'; // fill ultimate byte
	    free( area );
	} // for
    } // for

#ifndef ALLOCATOR					// uC++ allocator only
    // check calloc/realloc/free

    for ( i = 1; i < 10000; i += 12 ) {
	// initial N byte allocation
	char *area = (char *)calloc( 1, i );
	if ( area == NULL ) uAbort( "calloc/realloc/free out of memory" );
	if ( area[0] != '\0' || area[i - 1] != '\0' ||
	     area[malloc_usable_size( area ) - 1] != '\0' ||
	     ! malloc_zero_fill( area ) ) uAbort( "calloc/realloc/free corrupt storage1" );

	// Do not start this loop index at 0 because realloc of 0 bytes frees the storage.
	for ( int s = i; s < 256 * 1024; s += 26 ) {	// start at initial memory request
	    area = (char *)realloc( area, s );		// attempt to reuse storage
	    if ( area == NULL ) uAbort( "calloc/realloc/free out of memory" );
	    if ( area[0] != '\0' || area[s - 1] != '\0' ||
		 area[malloc_usable_size( area ) - 1] != '\0' ||
		 ! malloc_zero_fill( area ) ) uAbort( "calloc/realloc/free corrupt storage2" );
	} // for
	free( area );
    } // for

    // check memalign/realloc/free

    size_t amount = 2;
    for ( size_t a = uAlign(); a <= limit; a += a ) {	// generate powers of 2
	// initial N byte allocation
	char *area = (char *)memalign( a, amount );	// aligned N-byte allocation
      if ( area == NULL ) uAbort( "memalign/realloc/free out of memory" ); // no storage ?
	//cout << setw(6) << alignments[a] << " " << area << endl;
	if ( (size_t)area % a != 0 || malloc_alignment( area ) != a ) { // check for initial alignment
	    uAbort( "memalign/realloc/free bad alignment : memalign(%d,%d) = %p", (int)a, (int)amount, area );
	} // if
	area[0] = '\345'; area[amount - 2] = '\345';	// fill first/penultimate byte

	// Do not start this loop index at 0 because realloc of 0 bytes frees the storage.
	for ( int s = amount; s < 256 * 1024; s += 1 ) { // start at initial memory request
	    if ( area[0] != '\345' || area[s - 2] != '\345' ) uAbort( "memalign/realloc/free corrupt storage" );
	    area = (char *)realloc( area, s );		 // attempt to reuse storage
	  if ( area == NULL ) uAbort( "memalign/realloc/free out of memory" ); // no storage ?
	    //cout << setw(6) << i << " " << area << endl;
	    if ( (size_t)area % a != 0 ) {		// check for initial alignment
		uAbort( "memalign/realloc/free bad alignment %p", area );
	    } // if
	    area[s - 1] = '\345';			// fill last byte
	} // for
	free( area );
    } // for

    // check cmemalign/free

    for ( size_t a = uAlign(); a <= limit; a += a ) {	// generate powers of 2
	//cout << setw(6) << alignments[a] << endl;
	for ( int s = 1; s < 64 * 1024; s += 1 ) {	// allocation of size 0 can return NULL
	    char *area = (char *)cmemalign( a, 1, s );
	  if ( area == NULL ) uAbort( "cmemalign/free out of memory" );
	    //cout << setw(6) << i << " " << area << endl;
	    if ( (size_t)area % a != 0 || malloc_alignment( area ) != a ) { // check for initial alignment
		uAbort( "cmemalign/free bad alignment : cmemalign(%d,%d) = %p", (int)a, s, area );
	    } // if
	    if ( area[0] != '\0' || area[s - 1] != '\0' ||
		 area[malloc_usable_size( area ) - 1] != '\0' ||
		 ! malloc_zero_fill( area ) ) uAbort( "cmemalign/free corrupt storage" );
	    area[0] = '\345'; area[s - 1] = '\345';	// fill first/last byte
	    free( area );
	} // for
    } // for

    // check cmemalign/realloc/free

    amount = 2;
    for ( size_t a = uAlign() + uAlign(); a <= limit; a += a ) { // generate powers of 2
	// initial N byte allocation
	char *area = (char *)cmemalign( a, 1, amount );	// aligned N-byte allocation
      if ( area == NULL ) uAbort( "cmemalign/realloc/free out of memory" ); // no storage ?
	//cout << setw(6) << alignments[a] << " " << area << endl;
	if ( (size_t)area % a != 0 || malloc_alignment( area ) != a ) { // check for initial alignment
	    uAbort( "cmemalign/realloc/free bad alignment : cmemalign(%d,%d) = %p", (int)a, (int)amount, area );
	} // if
	if ( area[0] != '\0' || area[amount - 1] != '\0' ||
	     area[malloc_usable_size( area ) - 1] != '\0' ||
	     ! malloc_zero_fill( area ) ) uAbort( "cmemalign/realloc/free corrupt storage1" );
	area[0] = '\345'; area[amount - 2] = '\345';	// fill first/penultimate byte

	// Do not start this loop index at 0 because realloc of 0 bytes frees the storage.
	for ( int s = amount; s < 256 * 1024; s += 1 ) { // start at initial memory request
	    if ( area[0] != '\345' || area[s - 2] != '\345' ) uAbort( "cmemalign/realloc/free corrupt storage2" );
	    area = (char *)realloc( area, s );		// attempt to reuse storage
	    if ( area == NULL ) uAbort( "cmemalign/realloc/free out of memory" ); // no storage ?
	    //cout << setw(6) << i << " " << area << endl;
	    if ( (size_t)area % a != 0 || malloc_alignment( area ) != a ) { // check for initial alignment
		uAbort( "cmemalign/realloc/free bad alignment %p", area );
	    } // if
	    if ( area[s - 1] != '\0' || area[s - 1] != '\0' ||
		 area[malloc_usable_size( area ) - 1] != '\0' ||
		 ! malloc_zero_fill( area ) ) uAbort( "cmemalign/realloc/free corrupt storage3" );
	    area[s - 1] = '\345';			// fill last byte
	} // for
	free( area );
    } // for
#endif // ALLOCATION
    osacquire( cout ) << "worker " << &uThisTask() << " successful completion" << endl;
} // Worker::main


void uMain::main() {
    const unsigned int NoOfWorkers = 4;
    {
	uProcessor processors[NoOfWorkers - 1] __attribute__(( unused )); // more than one processor
	Worker workers[NoOfWorkers] __attribute__(( unused ));
    }
    malloc_stats();
} // uMain::main
