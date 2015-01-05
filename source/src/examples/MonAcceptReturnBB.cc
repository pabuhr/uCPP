//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 2004
// 
// MonAcceptReturnBB.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Feb 20 22:44:44 2004
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Nov 30 08:59:32 2005
// Update Count     : 17
// 


template<typename ELEMTYPE> _Monitor BoundedBuffer {
	const int size;										// number of buffer elements
	int front, back;									// position of front and back of queue
	int count;											// number of used elements in the queue
	ELEMTYPE *Elements;
  public:
	void insert( ELEMTYPE elem );
	ELEMTYPE remove();

	~BoundedBuffer() {
		delete [] Elements;
	} // BoundedBuffer::~BoundedBuffer

	BoundedBuffer( const int size = 10 ) : size( size ) {
		front = back = count = 0;
		Elements = new ELEMTYPE[size];
		uAcceptReturn( insert, ~BoundedBuffer );
	} // BoundedBuffer::BoundedBuffer

	_Nomutex int query() {
		return count;
	} // BoundedBuffer::query
}; // BoundedBuffer

template<typename ELEMTYPE> inline void BoundedBuffer<ELEMTYPE>::insert( ELEMTYPE elem ) {
	Elements[back] = elem;
	back = ( back + 1 ) % size;
	count += 1;

	if ( count == size ) {								// buffer full ?
		uAcceptReturn( remove, ~BoundedBuffer );		// only allow removals
	} else {
		uAcceptReturn( insert, remove, ~BoundedBuffer );
	} // if
} // BoundedBuffer::insert

template<typename ELEMTYPE> inline ELEMTYPE BoundedBuffer<ELEMTYPE>::remove() {
	ELEMTYPE elem;

	elem = Elements[front];
	front = ( front + 1 ) % size;
	count -= 1;

	if ( count == 0 ) {									// buffer empty ?
		uAcceptReturn( insert, ~BoundedBuffer ) elem;	// only allow insertions
	} else {
		uAcceptReturn( insert, remove, ~BoundedBuffer ) elem;
	} // if
} // BoundedBuffer::remove

#include "ProdConsDriver.i"

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ MonAcceptReturnBB.cc" //
// End: //
