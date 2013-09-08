//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// TaskAcceptBB.cc -- Generic bounded buffer using a task
// 
// Author           : Peter A. Buhr
// Created On       : Sun Sep 15 20:24:44 1991
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 31 18:50:16 2005
// Update Count     : 74
// 


template<typename ELEMTYPE> _Task BoundedBuffer {
	const int size;										// number of buffer elements
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
		Elements[back] = elem;
	} // BoundedBuffer::insert

	ELEMTYPE remove() {
		return Elements[front];
	} // BoundedBuffer::remove
  protected:
	void main() {
		for ( ;; ) {
			_Accept( ~BoundedBuffer )
				break;
			or _When ( count != size ) _Accept( insert ) {
				back = ( back + 1 ) % size;
				count += 1;
			} or _When ( count != 0 ) _Accept( remove ) {
				front = ( front + 1 ) % size;
				count -= 1;
			} // _Accept
		} // for
	} // BoundedBuffer::main
}; // BoundedBuffer

#include "ProdConsDriver.i"

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ TaskAcceptBB.cc" //
// End: //
