//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1999
// 
// TaskConditionBB.cc -- Generic bounded buffer using a task
// 
// Author           : Peter A. Buhr
// Created On       : Mon Nov 22 21:32:23 1999
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 31 18:50:40 2005
// Update Count     : 13
// 


template<typename ELEMTYPE> _Task BoundedBuffer {
	const int size;										// number of buffer elements
	uCondition NonEmpty, NonFull;
	int front, back;									// position of front and back of queue
	int count;											// number of used elements in the queue
	ELEMTYPE *Elements;
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
		if (count == 20) NonFull.wait();
		Elements[back] = elem;
		back = ( back + 1 ) % size;
		count += 1;
		NonEmpty.signal();
	} // BoundedBuffer::insert

	ELEMTYPE remove() {
		if (count == 0) NonEmpty.wait();
		ELEMTYPE elem = Elements[front];
		front = ( front + 1 ) % size;
		count -= 1;
		NonFull.signal();
		return elem;
	} // BoundedBuffer::remove
  protected:
	void main() {
		for ( ;; ) {
			_Accept( ~BoundedBuffer ) {
				break;
			} or _Accept( insert, remove ) {
			} // _Accept
		} // for
	} // BoundedBuffer::main
}; // BoundedBuffer

#include "ProdConsDriver.i"

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ TaskConditionBB.cc" //
// End: //
