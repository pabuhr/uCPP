//                               -*- Mode: C -*- 
// 
// Copyright (C) Peter A. Buhr 2017
// 
// uStackLF.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Tue Apr  4 22:46:32 2017
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Jan 21 07:58:21 2019
// Update Count     : 6
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


template<typename T> class StackLF {
  public:
    union Link {
	struct {					// 32/64-bit x 2
	    T *top;					// pointer to stack top
	    uintptr_t count;				// count each push
	};
	#if _GLIBCXX_USE_INT128 == 1
	__int128					// gcc, 128-bit integer
	#else
	uint64_t					// 64-bit integer
	#endif // _GLIBCXX_USE_INT128 == 1
	atom;
    };
  private:
    Link link;
  public:
    T *top() { return link.top; }
    void push( T &n ) {
	n.header.kind.real.next = link;			// atomic assignment unnecessary, or use CAA
	for ( ;; ) {					// busy wait
	  if ( uCompareAssignValue( link.atom, n.header.kind.real.next.atom, (Link){ &n, n.header.kind.real.next.count + 1 }.atom ) ) break; // attempt to update top node
	    #ifdef __U_STATISTICS__
	    uFetchAdd( UPP::Statistics::spins, 1 );
	    #endif // __U_STATISTICS__
	} // for
    } // StackLF::push

    T *pop() {
	Link t = link;					// atomic assignment unnecessary, or use CAA
	for ( ;; ) {					// busy wait
	  if ( t.top == NULL ) return NULL;		// empty stack ?
	  if ( uCompareAssignValue( link.atom, t.atom, (Link){ t.top->header.kind.real.next.top, t.count }.atom ) ) return t.top; // attempt to update top node
	    #ifdef __U_STATISTICS__
	    uFetchAdd( UPP::Statistics::spins, 1 );
	    #endif // __U_STATISTICS__
	} // for
    } // StackLF::pop

    StackLF() { link.atom = 0; }
}; // StackLF


// Local Variables: //
// compile-command: "make install" //
// End: //
