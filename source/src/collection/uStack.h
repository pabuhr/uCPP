//                              -*- Mode: C++ -*- 
// 
// Copyright (C) Glen Ditchfield 1994
// 
// uStack.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sun Feb 13 19:35:33 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Aug  2 15:36:15 2012
// Update Count     : 64
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

#ifndef __U_STACK_H__
#define __U_STACK_H__


#include "uCollection.h"

// A uStack<T> is a uCollection<T> that defines an ordering among the elements:
// they are returned by drop() in the reverse order that they are added by
// add().

// The implementation is a typical singly-linked list, except that uStack
// maintains uColable's invariant by having the next field of the last element
// of the list point to itself instead of being null.

template<typename T> class uStack: public uCollection<T> {
  protected:
    using uCollection<T>::root;

    uStack(const uStack&);				// no copy
    uStack &operator=(const uStack&);			// no assignment
  public:
    using uCollection<T>::head;
    using uCollection<T>::uNext;

    uStack() : uCollection<T>() {}			// post: isEmpty().
    inline T *top() const {
	return head();
    }
    void addHead( T *n ) {
#ifdef __U_DEBUG__
	if ( n->listed() ) uAbort( "(uStack &)%p.addHead( %p ) node is already on another list.", this, n );
#endif // __U_DEBUG__
	uNext(n) = root ? root : n;
	root = n;
    }
    inline void add( T *n ) {
	addHead( n );
    }
    inline void push( T *n ) {
	addHead( n );
    }
    T *drop() {
	T *t = root;
	if (root) {
	    root = ( T *)uNext(root);
	    if (root == t) root = 0;			// There was only one element.
	    uNext(t) = 0;
	} // if
	return t;
    }
    inline T *pop() {
	return drop();
    }
};


// A uStackIter<T> is a subclass of uColIter<T> that generates the elements of a
// uStack<T>.  It returns the elements in the order that they would be returned
// by drop().

template<typename T> class uStackIter : public uColIter<T> {
  protected:
    using uColIter<T>::curr;
    using uColIter<T>::uNext;
  public:
    uStackIter() : uColIter<T>() {}			// post: elts = null.
    // Create a iterator active in stack s.
    uStackIter(const uStack<T> &s) {
	curr = s.head();
    }
    // Make the iterator active in stack s.
    void over(const uStack<T> &s) {			// post: elts = {e in s}.
	curr = s.head();
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


#endif // __U_STACK_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
