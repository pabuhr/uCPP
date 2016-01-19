//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 2015
// 
// MutexCondBB.cc -- Generic bounded buffer problem using a mutex lock and condition variables
// 
// Author           : Peter A. Buhr
// Created On       : Sun May  3 23:11:32 2015
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue May  5 08:56:29 2015
// Update Count     : 7
// 

template<typename ELEMTYPE> class BoundedBuffer {
	const int size;										// number of buffer elements
	int front, back;									// position of front and back of queue
	int count;											// number of used elements in the queue
	uOwnerLock mutex;
	uCondLock BufFull, BufEmpty;
	ELEMTYPE *Elements;
  public:
	BoundedBuffer( const unsigned int size = 10 ) : size( size ) {
		front = back = count = 0;
		Elements = new ELEMTYPE[size];
	} // BoundedBuffer::BoundedBuffer

	~BoundedBuffer() {
		delete [] Elements;
	} // BoundedBuffer::~BoundedBuffer

	int query() {
		return count;
	} // BoundedBuffer::query

	void insert( ELEMTYPE elem ) {
		mutex.acquire();
		while ( count == size ) BufFull.wait( mutex );

		assert( count < size );
		Elements[back] = elem;
		back = ( back + 1 ) % size;
		count += 1;

		BufEmpty.signal();
		mutex.release();
	}; // BoundedBuffer::insert

	ELEMTYPE remove() {
		mutex.acquire();
		while ( count == 0 ) BufEmpty.wait( mutex );

		assert( count > 0 );
		ELEMTYPE elem = Elements[front];
		front = ( front + 1 ) % size;
		count -= 1;

		BufFull.signal();
		mutex.release();
		return elem;
	} // BoundedBuffer::remove
}; // BoundedBuffer

#include "ProdConsDriver.i"

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ MutexCondBB.cc" //
// End: //
