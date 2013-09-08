//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// SemaphoreBB.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Aug 15 16:42:42 1991
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 31 18:48:08 2005
// Update Count     : 54
// 

#include <uSemaphore.h>

template<typename ELEMTYPE> class BoundedBuffer {
	const int size;										// number of buffer elements
	int front, back;									// position of front and back of queue
	uSemaphore full, empty;								// synchronize for full and empty BoundedBuffer
	uSemaphore ilock, rlock;							// insertion and removal locks
	ELEMTYPE *Elements;

	BoundedBuffer( BoundedBuffer & );					// no copy
	BoundedBuffer &operator=( BoundedBuffer & );		// no assignment
  public:
	BoundedBuffer( const int size = 10 ) : size( size ), full( 0 ), empty( size ) {
		front = back = 0;
		Elements = new ELEMTYPE[size];
	} // BoundedBuffer::BoundedBuffer

	~BoundedBuffer() {
		delete  Elements;
	} // BoundedBuffer::~BoundedBuffer

	void insert( ELEMTYPE elem ) {
		empty.P();										// wait if queue is full

		ilock.P();										// serialize insertion
		Elements[back] = elem;
		back = ( back + 1 ) % size;
		ilock.V();

		full.V();										// signal a full queue space
	} // BoundedBuffer::insert

	ELEMTYPE remove() {
		ELEMTYPE elem;
		
		full.P();										// wait if queue is empty

		rlock.P();										// serialize removal
		elem = Elements[front];
		front = ( front + 1 ) % size;
		rlock.V();

		empty.V();										// signal empty queue space
		return elem;
	} // BoundedBuffer::remove
}; // BoundedBuffer

#include "ProdConsDriver.i"

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ SemaphoreBB.cc" //
// End: //
