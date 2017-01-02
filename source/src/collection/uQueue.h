//                              -*- Mode: C++ -*- 
// 
// Copyright (C) Glen Ditchfield 1994
// 
// uQueue.h -- 
// 
// Author           : Glen Ditchfield
// Created On       : Sun Feb 13 17:35:59 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Oct 11 21:57:23 2016
// Update Count     : 115
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


#ifndef __U_QUEUE_H__
#define __U_QUEUE_H__


#include "uCollection.h"

// A uQueue<T> is a uCollection<T> that defines an ordering among the elements:
// they are returned by drop() in the same order that they are added by add().

// The implementation is a typical singly-linked list, except that uQueue
// maintains uColable's invariant by having the next field of the last element
// of the list point to itself instead of being null.

template<typename T> class uQueue : public uCollection<T> {
  protected:
    using uCollection<T>::root;

    T *last;						// last element, or 0 if queue is empty.
  public:
    uQueue( const uQueue & ) = delete;			// no copy
    uQueue( uQueue && ) = delete;
    uQueue &operator=( const uQueue & ) = delete;	// no assignment

    using uCollection<T>::empty;
    using uCollection<T>::head;
    using uCollection<T>::uNext;

    inline uQueue() {					// post: isEmpty().
	last = 0;
    }
    inline T *tail() const {
	return last;
    }
    inline T *succ( T *n ) const {			// pre: *n in *this
#ifdef __U_DEBUG__
	if ( ! n->listed() ) uAbort( "(uQueue &)%p.succ( %p ) : Node is not on a list.", this, n );
#endif // __U_DEBUG__
	return (uNext(n) == n) ? 0 : (T *)uNext(n);
    }							// post: n == tail() & succ(n) == 0 | n != tail() & *succ(n) in *this
    void addHead( T *n ) {
#ifdef __U_DEBUG__
	if ( n->listed() ) uAbort( "(uQueue &)%p.addHead( %p ) : Node is already on another list.", this, n );
#endif // __U_DEBUG__
	if (last) {
	    uNext(n) = root;
	    root = n;
	} else {
	    last = root = n;
	    uNext(n) = n;				// last node points to itself
	}
    }
    void addTail( T *n ) {
#ifdef __U_DEBUG__
	if ( n->listed() ) uAbort( "(uQueue &)%p.addTail( %p ) : Node is already on another list.", this, n );
#endif // __U_DEBUG__
	if (last) uNext(last) = n;
	else root = n;
	last = n;
	uNext(n) = n;					// last node points to itself
    }
    inline void add( T *n ) {
	addTail( n );
    }
    T *dropHead() {
	T *t = head();
	if (root) {
	    root = (T *)uNext(root);
	    if (root == t) {
		root = last = 0;			// only one element
	    }
	    uNext(t) = 0;
	}
	return t;
    }
    inline T *drop() {
	return dropHead();
    }
    inline T *dropTail() {				// O(n)
	T *n = tail();
	return n ? remove( n ), n : 0;
    }
    void remove( T *n ) {				// O(n)
#ifdef __U_DEBUG__
	if ( ! n->listed() ) uAbort( "(uQueue &)%p.remove( %p ) : Node is not on a list.", this, n );
#endif // __U_DEBUG__
	T *prev = 0;
	T *curr = root;
	for ( ;; ) {
	    if (n == curr) {				// found => remove
		if (root == n) {
		    dropHead();
		} else if (last == n) {
		    last = prev;
		    uNext(last) = last;
		} else {
		    uNext(prev) = uNext(curr);
		}
		uNext(n) = 0;
		break;
	    }
#ifdef __U_DEBUG__
	    // not found => error
	    if (curr == last) uAbort( "(uQueue &)%p.remove( %p ) : Node is not in list.", this, n );
#endif // __U_DEBUG__
	    prev = curr;
	    curr = (T *)uNext(curr);
	}
    }							// post: !n->listed().
    // Transfer the "from" list to the end of this sequence; the "from" list is empty after the transfer.
    void transfer( uQueue<T> &from ) {
	if ( from.empty() ) return;			// "from" list empty ?
	if ( empty() ) {				// "to" list empty ?
	    root = from.root;
	} else {					// "to" list not empty
	    uNext(last) = from.root;
	}
	last = from.last;
	from.root = from.last = 0;			// mark "from" list empty
    }
    // Transfer the "from" list up to node "n" to the end of this list; the "from" list becomes the list after node "n".
    // Node "n" must be in the "from" list.
    void split( uQueue<T> &from, T *n ) {
#ifdef __U_DEBUG__
	if ( ! n->listed() ) uAbort( "(uQueue &)%p.split( %p ) : Node is not on a list.", this, n );
#endif // __U_DEBUG__
	uQueue<T> to;
	to.root = from.root;				// start of "to" list
	to.last = n;					// end of "to" list
	from.root = (T *)uNext( n );			// start of "from" list
	if ( n == from.root ) {				// last node in list ?
	    from.root = from.last = 0;			// mark "from" list empty
	} else {
	    uNext( n ) = n;				// fix end of "to" list
	}
	transfer( to );
    }
};


// A uQueueIter<T> is a subclass of uColIter<T> that generates the elements of a
// uQueue<T>.  It returns the elements in the order that they would be returned
// by drop().

template<typename T> class uQueueIter : public uColIter<T> {
  protected:
    using uColIter<T>::curr;
  public:
    uQueueIter():uColIter<T>() {}			// post: elts = null.
    // Create an iterator active in queue q.
    inline uQueueIter( const uQueue<T> &q ) {		// post: elts = {e in q}.
	curr = q.head();
    }
    // Make the iterator active in queue q.
    inline void over( const uQueue<T> &q ) {		// post: elts = {e in q}.
	curr = q.head();
    }
    bool operator>>( T *&tp ) {
	if (curr) {
	    tp = curr;
	    T *n = (T *)uNext(curr);
	    curr = (n == curr) ? 0 : n;
	} else tp = 0;
	return tp != 0;
    }
};


#endif // __U_QUEUE_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
