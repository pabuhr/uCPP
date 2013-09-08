//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// ProdConsDriver.i -- Producer/Consumer Driver for a bounded buffer
// 
// Author           : Peter A. Buhr
// Created On       : Sun Sep 15 18:19:38 1991
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Mar 27 08:21:50 2007
// Update Count     : 75
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Task producer {
	BoundedBuffer<int> &buf;

	void main() {
		const int NoOfItems = rand() % 20;
		int item;

		for ( int i = 1; i <= NoOfItems; i += 1 ) {		// produce a bunch of items
			yield( rand() % 20 );						// pretend to spend some time producing
			item = rand() % 100 + 1;					// produce a random number
			osacquire( cout ) << "Producer:" << this << ", value:" << item << endl;
			buf.insert( item );							// insert element into queue
		} // for
		osacquire( cout ) << "Producer " << this << " is finished!" << endl;
	} // producer::main
  public:
	producer( BoundedBuffer<int> &buf ) : buf( buf ) {
	} // producer::producer
}; // producer

_Task consumer {
	BoundedBuffer<int> &buf;

	void main() {
		int item;

		for ( ;; ) {									// consume until a negative element appears
			item = buf.remove();						// remove from front of queue
			osacquire( cout ) << "Consumer:" << this << ", value:" << item << endl;
		  if ( item == -1 ) break;
			yield( rand() % 20 );						// pretend to spend some time consuming
		} // for
		osacquire( cout ) << "Consumer " << this << " is finished!" << endl;
	} // consumer::main
  public:
	consumer( BoundedBuffer<int> &buf ) : buf( buf ) {
	} // consumer::consumer
}; // consumer

void uMain::main() {
	const int NoOfCons = 2, NoOfProds = 3;
	BoundedBuffer<int> buf;								// create a buffer monitor
	consumer *cons[NoOfCons];							// pointer to an array of consumers
	producer *prods[NoOfProds];							// pointer to an array of producers

	for ( int i = 0; i < NoOfCons; i += 1 ) {			// create consumers
	    cons[i] = new consumer( buf );
	} // for
	for ( int i = 0; i < NoOfProds; i += 1 ) {			// create producers
	    prods[i] = new producer( buf );
	} // for

	for ( int i = 0; i < NoOfProds; i += 1 ) {			// wait for producers to end
	    delete prods[i];
	} // for
	for ( int i = 0; i < NoOfCons; i += 1 ) {			// terminate each consumer
		buf.insert( -1 );
	} // for
	for ( int i = 0; i < NoOfCons; i += 1 ) {			// wait for consumers to end
	    delete cons[i];
	} // for

	cout << "successful completion" << endl;
} // uMain::main

// Local Variables: //
// tab-width: 4 //
// End: //
