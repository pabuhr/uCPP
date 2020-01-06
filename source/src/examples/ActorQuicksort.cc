// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2016
// 
// ActorQuickSort.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Tue Oct  1 21:35:39 2019
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Jan  6 08:43:01 2020
// Update Count     : 57
// 
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
#include <fstream>
#include <sstream>
#include <cmath>
using namespace std;
//#define TIMING
#include <chrono>
using namespace chrono;
#include <uActor.h>

template<typename T> _Actor Quicksort {
    T * values;						// communication variables
    int low, high, depth;

    void sort( T values[], int low, int high, int depth ) {
	int left, right;				// index to left/right-hand side of the values
	T pivot;					// pivot value of values
	T swap;						// temporary

	//osacquire( cout ) << this << " QS(" << low << ", " << high << ", " << depth << ")" << endl;

	uThisTask().verify();				// check for stack overflow due to recursion

	// partition while 2 or more elements in the array
	if ( low < high ) {
	    pivot = values[low + ( high - low ) / 2];
	    left  = low;
	    right = high;

	    // partition: move values less < pivot before the pivot and values > pivot after the pivot
	    do {
		while ( values[left] < pivot ) left += 1; // changed values[left] < pivot
		while ( pivot < values[right] ) right -= 1;
		if ( left <= right ) {
		    swap = values[left];		// interchange values
		    values[left]  = values[right];
		    values[right] = swap;
		    left += 1;
		    right -= 1;
		} // if
	    } while ( left <= right );

	    // restrict number of tasks to slightly greater than number of processors
	    if ( depth > 0 ) {
		depth -= 1;
	        *new Quicksort( values, low, right, depth ) | uActor::startMsg; // concurrently sort lower half
	        *new Quicksort( values, left, high, depth ) | uActor::startMsg; // concurrently sort upper half
	    } else {
		sort( values, low, right, 0 );		// sequentially sort lower half
		sort( values, left, high, 0 );		// sequentially sort upper half
	    } // if
	} // if
    } // Quicksort::sort

    uActor::Allocation receive( uActor::Message &msg ) {
#ifdef TIMING
	high_resolution_clock::time_point start;
	if ( depth == 0 ) {
	    start = high_resolution_clock::now();
	} // if
#endif // TIMING
	sort( values, low, high, depth );
#ifdef TIMING
	if ( depth == 0 ) {
	    duration<double> time = high_resolution_clock::now() - start;
	    osacquire( cout ) << time.count() << endl;
	} // if
#endif // TIMING
	return uActor::Delete;
    } // Quicksort::receive

    Quicksort( T values[], int low, int high, int depth ) :
	values( values ), low( low ), high( high ), depth( depth ) {
    } // Quicksort::Quicksort
  public:
    Quicksort( T values[], int size, int depth ) :
	values( values ), low( 0 ), high( size ), depth( depth ) {
    } // Quicksort::Quicksort
}; // Quicksort


bool convert( int & val, const char * buffer ) {	// convert C string to integer
    stringstream ss( buffer );				// connect stream and buffer
    string temp;
    ss >> dec >> val;					// convert integer from buffer
    return ! ss.fail() &&				// conversion successful ?
	! ( ss >> temp );				// characters after conversion all blank ?
} // convert

void usage( char * argv[] ) {
    cerr << "Usage: " << argv[0] << " ( unsorted-file [ sorted-file ] | -t size (>= 0) [ depth (>= 0) ] )" << endl;
    exit( EXIT_FAILURE );				// TERMINATE!
} // usage

int main( int argc, char * argv[] ) {
    istream * unsortedfile = nullptr;
    ostream * sortedfile = &cout;			// default value
    // Must be signed because of the conversion routine.
    int depth = 0, size;

    if ( argc < 2 || argc > 4 ) usage( argv );		// wrong number of options
    if ( strcmp( argv[1], "-t" ) == 0 ) {
	switch ( argc ) {
	  case 4:
	    if ( ! convert( depth, argv[3] ) || depth < 0 ) usage( argv );
	    // FALL THROUGH
	  case 3:
	    if ( ! convert( size, argv[2] ) || size < 0 ) usage( argv );
	} // switch
    } else {
	switch ( argc ) {
	  case 3:
	    try {
		sortedfile = new ofstream( argv[2] );	// open the output file
	    } catch( uFile::Failure & ) {
		cerr << "Error! Could not open sorted output file \"" << argv[2] << "\"" << endl;
		usage( argv );
	    } // try
	    // FALL THROUGH
	  case 2:
	    try {
		unsortedfile = new ifstream( argv[1] );	// open the input file
	    } catch( uFile::Failure & ) {
		cerr << "Error! Could not open unsorted input file \"" << argv[1] << "\"" << endl;
		usage( argv );
	    } // try
	} // switch
    } // if

    enum { ValuesPerLine = 22 };

    if ( unsortedfile != nullptr ) {			// generate output ?
	for ( ;; ) {
	    *unsortedfile >> size;			// read number of elements in the list
	  if ( unsortedfile->fail() ) break;
	    TYPE * values = new TYPE[size];		// values to be sorted, too large to put on stack
	    for ( int counter = 0; counter < size; counter += 1 ) { // read unsorted numbers
		*unsortedfile >> values[counter];
		if ( counter != 0 && counter % ValuesPerLine == 0 ) *sortedfile << endl << "  ";
		*sortedfile << values[counter];
		if ( counter < size - 1 && (counter + 1) % ValuesPerLine != 0 ) *sortedfile << ' ';
	    } // for
	    *sortedfile << endl;
	    if ( size > 0 ) {				// values to sort ?
		uActorStart();				// start actor system
		Quicksort<TYPE> * qs = new Quicksort<TYPE>( values, size - 1, 0 ); // sort values
		*qs | uActor::startMsg;
		uActorStop();				// stop actor system
	    } // wait until sort tasks terminate
	    for ( int counter = 0; counter < size; counter += 1 ) { // print sorted list
		if ( counter != 0 && counter % ValuesPerLine == 0 ) *sortedfile << endl << "  ";
		*sortedfile << values[counter];
		if ( counter < size - 1 && (counter + 1) % ValuesPerLine != 0 ) *sortedfile << ' ';
	    } // for
	    *sortedfile << endl << endl;

	    delete [] values;
	} // for
	delete unsortedfile;				// close input/output files
	if ( sortedfile != &cout ) delete sortedfile;
    } else {
	//cout << size << endl;
	TYPE * values = new TYPE[size];			// values to be sorted, too large to put on stack
	for ( int counter = 0; counter < size; counter += 1 ) { // generate unsorted numbers
	    values[counter] = size - counter;		// descending values
	} // for
	for ( int i = 0; i < 200; i +=1 ) {		// random shuffle a few values
	    swap( values[rand() % size], values[rand() % size] );
	} // for
	{
	    uProcessor p[(1 << depth) - 1];
	    uActorStart();				// start actor system
	    uTime start = uClock::currTime();
	    *new Quicksort<TYPE>( values, size - 1, depth ) | uActor::startMsg; // sort values
	    uActorStop();				// stop actor system
	    cout << uClock::currTime() - start << endl;
	} // wait until sort tasks terminate

	// for ( int counter = 0; counter < size - 1; counter += 1 ) { // check sorting
	//     if ( values[counter] > values[counter + 1] ) abort();
	// } // for

	delete [] values;
    } // if

    //cout << qthreads << endl;
} // main

// for depth in 0 1 2 3 4 5 ; do echo "sort 500000000 values with ${depth} depth" ; time -f "%Uu %Ss %E %Mkb" a.out -t 500000000 ${depth} ; done

// Local Variables: //
// compile-command: "u++-work -Wall -g -O2 -multi -nodebug -DTYPE=int ActorQuicksort.cc" //
// End: //
