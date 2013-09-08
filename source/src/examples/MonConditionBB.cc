//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// MonConditionBB.cc -- Generic bounded buffer problem using a monitor and condition variables
// 
// Author           : Peter A. Buhr
// Created On       : Thu Aug  2 11:35:05 1990
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Nov 30 08:41:43 2005
// Update Count     : 57
// 


template<typename ELEMTYPE> _Monitor BoundedBuffer {
	const int size;										// number of buffer elements
	int front, back;									// position of front and back of queue
	int count;											// number of used elements in the queue
	ELEMTYPE *Elements;
	uCondition BufFull, BufEmpty;
  public:
	BoundedBuffer( const int size = 10 ) : size( size ) {
		front = back = count = 0;
		Elements = new ELEMTYPE[size];
	} // BoundedBuffer::BoundedBuffer

	~BoundedBuffer() {
		delete [] Elements;
	} // BoundedBuffer::~BoundedBuffer

	_Nomutex int query() {
		return count;
	} // BoundedBuffer::query

	void insert( ELEMTYPE elem ) {
		if ( count == size ) {
			BufFull.wait();
		} // if

		Elements[back] = elem;
		back = ( back + 1 ) % size;
		count += 1;

		BufEmpty.signal();
	}; // BoundedBuffer::insert
	
	ELEMTYPE remove() {
		ELEMTYPE elem;

		if ( count == 0 ) {
			BufEmpty.wait();
		} // if

		elem = Elements[front];
		front = ( front + 1 ) % size;
		count -= 1;

		BufFull.signal();
		return elem;
	}; // BoundedBuffer::remove
}; // BoundedBuffer

#include "ProdConsDriver.i"

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ MonConditionBB.cc" //
// End: //
