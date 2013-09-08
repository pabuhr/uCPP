//                              -*- Mode: C++ -*- 
// 
// Copyright (C) Richard C. Bilson 2003
// 
// uBitSet.h -- Fast bit-set operations
// 
// Author           : Richard C. Bilson and Peter A. Buhr
// Created On       : Mon Dec 15 14:05:51 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat May  2 11:54:05 2009
// Update Count     : 18
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

#ifndef __U_BITSET_H__
#define __U_BITSET_H__

#pragma __U_NOT_USER_CODE__


#include <uStaticAssert.h>				// _STATIC_ASSERT_
#include <assert.h>					// assert

#include <cstring>					// ffs, memset


template<int nbits> class uBitSet {
    _STATIC_ASSERT_( nbits > 0 );
    
    typedef int BaseType;
    _STATIC_ASSERT_( sizeof( BaseType ) == 4 );

    enum { idxshift = 5, idxmask = 0x1f,
           nbase = ( ( nbits - 1 ) >> idxshift ) + 1 };
    BaseType bits[ nbase ];
  public:
    void set( int idx ) {
	assert( idx >= 0 && idx < nbits );
	bits[ idx >> idxshift ] |= ( 1u << ( idx & idxmask ) );
    } // uBitSet::set

    void clr( int idx ) {
	assert( idx >= 0 && idx < nbits );
	bits[ idx >> idxshift ] &= ~( 1u << ( idx & idxmask ) );
    } // uBitSet::clr

    void setAll() {
	memset( bits, -1, nbase * sizeof( BaseType ) );	// assumes 2's complement
    } // uBitSet::setAll

    void clrAll() {
	memset( bits, 0, nbase * sizeof( BaseType ) );
    } // uBitSet::clrAll

    bool isSet( int idx ) const {
	assert( idx >= 0 && idx < nbits );
	return bits[ idx >> idxshift ] & ( 1u << ( idx & idxmask ) );
    } // uBitSet::is Set

    bool isAllClr() const {
	int elt;
	for ( elt = 0; elt < nbase; elt += 1 ) {
	  if ( bits[ elt ] != 0 ) break;
	} // for
	return elt == nbase;
    } // uBitSet::isAllClr

    int findFirstSet() const {
	int elt;
	for ( elt = 0;; elt += 1 ) {
	  if ( elt >= nbase ) return -1;
	  if ( bits[ elt ] != 0 ) break;
	} // for
	return ffs( bits[ elt ] ) - 1 + ( elt << idxshift );
    } // uBitSet::findFirstSet

    int operator[]( int i ) const {
	assert( i >= 0 && i < nbase );
	return bits[ i ];
    } // uBitSet::operator[]
}; // uBitSet


// Special optimization when the number of bits in __U_MAXENTRYBITS__ is a long.

template<> class uBitSet< sizeof( long ) * 8 > {
    unsigned long bits;
    enum { nbits = sizeof( long ) * 8 };
  public:
    void set( int idx ) {
	assert( idx >= 0 && idx < nbits );
	bits |= ( 1ul << idx );
    } // uBitSet::set

    void clr( int idx ) {
	assert( idx >= 0 && idx < nbits );
	bits &= ~( 1ul << idx );
    } // uBitSet::clr

    void setAll() {
	bits = (unsigned long)-1;
    } // uBitSet::setAll

    void clrAll() {
	bits = 0;
    } // uBitSet::clrAll

    bool isSet( int idx ) const {
	assert( idx >= 0 && idx < nbits );
	return bits & ( 1ul << idx );
    } // uBitSet::is Set

    bool isAllClr() const {
	return bits == 0;
    } // uBitSet::isAllClr

    int findFirstSet() const {
	return ffs( bits );
    } // uBitSet::findFirstSet

    int operator[]( int i ) const {
	assert( i == 0 );
	return bits;
    } // uBitSet::operator[]
}; // uBitSet


#endif // __U_BITSET_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
