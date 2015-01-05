//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// BinaryInsertionSort.cc -- Binary Insertion Sort, semi-coroutines
// 
// Author           : Peter A. Buhr
// Created On       : Thu Aug  2 11:53:37 1990
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Sep 26 01:37:10 2012
// Update Count     : 62
// 

#include <iostream>
using std::cout;
using std::endl;

_Coroutine BinarySort {
  private:
	int in, out;
	void main();
  public:
	void input( int );
	int output();
}; // BinarySort

void BinarySort::main() {
	int pivot;

	pivot = in;											// first value is the pivot value
	if ( pivot == -1 ) {								// no data values
		suspend();										// acknowledge end of input
		out = -1;
		return;											// terminate output
	} // if

	BinarySort less, greater;							// create siblings

	for ( ;; ) {
		suspend();										// get more input
	  if ( in == -1 ) break;
		if ( in <= pivot ) {							// direct value along appropriate branch
			less.input( in );
		} else {
			greater.input( in );
		} // if
	} // for

	less.input( -1 );									// terminate input
	greater.input( -1 );								// terminate input
	suspend();											// acknowledge end of input

	// return sorted values

	for ( ;; ) {
		out = less.output();							// retrieve the smaller values
	  if ( out == -1 ) break;							// no more smaller values ?
		suspend();										// return smaller values
	} // for

	out = pivot;
	suspend();											// return the pivot

	for ( ;; ) {
		out = greater.output();							// retrieve the larger values
	  if ( out == -1 ) break;							// no more larger values ?
		suspend();										// return larger values
	} // for

	out = -1;
	return;												// terminate output
} // BinarySort::main

void BinarySort::input( int val ) {
	in = val;
	resume();
} // BinarySort::input

int BinarySort::output() {
	resume();
	return out;
} // BinarySort::output

void uMain::main() {
	const int NoOfValues = 40;
	BinarySort bs;
	int value;
	int i;
	
	// sort values

	cout << "unsorted values:" << endl;
	for ( i = 1; i <= NoOfValues; i += 1 ) {
		value = rand() % 100;
		cout << value << " ";
		bs.input( value );
	} // for
	cout << endl;
	bs.input( -1 );

	// retrieve sorted values

	cout << "sorted values:" << endl;
	for ( ;; ) {
		value = bs.output();							// retrieve values
	  if ( value == -1 ) break;							// no more values ?
		cout << value << " ";							// print values
	} // for
	cout << endl;
} // uMain::main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ BinaryInsertionSort.cc" //
// End: //
