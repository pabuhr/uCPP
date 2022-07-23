//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1995
// 
// uBarrier.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sat Sep 16 20:56:38 1995
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Apr 19 11:28:14 2022
// Update Count     : 53
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


#pragma once


_Mutex _Coroutine uBarrier {
	uCondition Waiters;
	unsigned int Total, Count;

	void init( unsigned int total ) {
		Count = 0;
		Total = total;
	} // uBarrier::init
  protected:
	void main() {
		for ( ;; ) {
			suspend();
		} // for
	} // uBarrier::main

	virtual void last() {								// called by last task to reach the barrier
		resume();
	} // uBarrier::last
  public:
	uBarrier() {										// for use with reset
		init( 0 );
	} // uBarrier::uBarrier

	uBarrier( unsigned int total ) {
		init( total );
	} // uBarrier::uBarrier

	virtual ~uBarrier() {
	} // uBarrier::~uBarrier

	_Nomutex unsigned int total() const {				// total participants in the barrier
		return Total;
	} // uBarrier::total

	_Nomutex unsigned int waiters() const {				// number of waiting tasks
		return Count;
	} // uBarrier::waiters

	void reset( unsigned int total ) {
#ifdef __U_DEBUG__
		if ( Count != 0 ) {
			abort( "(uBarrier &)%p.reset( %d ) : Attempt to reset barrier total while tasks blocked on barrier.", this, total );
		} // if
#endif // __U_DEBUG__
		init( total );
	} // uBarrier::reset

	virtual void block() {
		Count += 1;
		if ( Count < Total ) {							// all tasks arrived ?
			Waiters.wait();
		} else {
			last();										// call the last routine
			for ( ; ! Waiters.empty(); ) {				// restart all waiting tasks
				Waiters.signal();						// LIFO release, N-1 cxt switches
//				Waiters.signalBlock();					// FIFO release, 2N cxt switches
			} // for
		} // if
		Count -= 1;
	} // uBarrier::block
}; // uBarrier


// Local Variables: //
// compile-command: "make install" //
// End: //
