//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2024
// 
// Array.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Jun 13 14:39:12 2024
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 13 17:54:23 2025
// Update Count     : 8
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

#include <iostream>
#include <algorithm>
using namespace std;

struct S {												// no default constructor
	int i, j;
	S( int i ) : i{i}, j{i} {}
	S( int i, int j ) : i{i}, j{j} {}
	~S() { cout << "S "; }
};
ostream & operator<<( ostream & os, const S & s ) {
	return os << s.i << ',' << s.j;
}

struct T {												// no default constructor
	const int i, j;										// const fields
	T( int i, int j ) : i{i}, j{j} {}
	~T() { cout << "T "; }
};
ostream & operator<<( ostream & os, const T & t ) {
	return os << t.i << ',' << t.j;
}

template< typename T > void f( uArrayRef( T, parm ) ) {	// pass uArray by reference
	for ( size_t i = 0; i < parm.size(); i += 1 ) cout << *parm[i] << ' ';
	cout << endl;
}

int main() {
	{
		uArrayFill( int, iarr, 10, 5 );
		for ( size_t i = 0; i < iarr.size(); i += 1 ) cout << *iarr[i] << ' ';
		cout << endl;
		for ( size_t i = 0; i < iarr.size(); i += 1 ) iarr[i] = i;
		for ( size_t i = 0; i < iarr.size(); i += 1 ) cout << *iarr[i] << ' ';
		cout << endl;
		f( iarr );

		uArray( int, iarr2, 11 );
		iarr = iarr2 = iarr;
		for ( size_t i = 0; i < iarr2.size(); i += 1 ) cout << *iarr2[i] << ' ';
		cout << endl;
	}
	{
		uArrayFill( string, sarr, 10, "xyz" );			//  must call default constructor
		for ( size_t i = 0; i < sarr.size(); i += 1 ) cout << *sarr[i] << ' ';
		cout << endl;
		for ( size_t i = 0; i < sarr.size(); i += 1 ) sarr[i] = "abc";
		for ( size_t i = 0; i < sarr.size(); i += 1 ) cout << *sarr[i] << ' ';
		cout << endl;
		f( sarr );
	}
	{
		uArrayPtr( string, sarr2, 10 );
		for ( size_t i = 0; i < sarr2.size(); i += 1 ) sarr2[i] = "abc";
		for ( size_t i = 0; i < sarr2.size(); i += 1 ) cout << *sarr2[i] << ' ';
		cout << endl;
		f( sarr2 );
	}
	{
		uArrayFill( S, arr2, 10, 5  );
		for ( size_t i = 0; i < arr2.size(); i += 1 ) cout << arr2[i]->i << ',' << arr2[i]->j << ' ';
		cout << endl;
		for ( size_t i = 0; i < arr2.size(); i += 1 ) arr2[i]( 2, 3 );
		for ( size_t i = 0; i < arr2.size(); i += 1 ) cout << arr2[i]->i << ',' << arr2[i]->j << ' ';
		cout << endl;
		for ( size_t i = 0; i < arr2.size(); i += 1 ) arr2[i] = (S){ (int)i, 3 };
		for ( size_t i = 0; i < arr2.size(); i += 1 ) cout << arr2[i]->i << ' ';
		cout << endl;
		for ( size_t i = 0; i < arr2.size(); i += 1 ) arr2[i] = S{ (int)i, 3 };
		for ( size_t i = 0; i < arr2.size(); i += 1 ) cout << arr2[i]->i << ' ';
		cout << endl;
		f( arr2 );
	}
	{
		uArray( T, arr3, 10 );							// const => no assignment
		for ( size_t i = 0; i < arr3.size(); i += 1 ) arr3[i]( 3, 4 );
		for ( size_t i = 0; i < arr3.size(); i += 1 ) cout << arr3[i]->i << ',' << arr3[i]->j << ' ';
		cout << endl;
		f( arr3 );
	}
	{
		struct E {};
		try {
			uArrayFill( S, arr1, 10, 2, 3 );
			for ( size_t i = 0; i < arr1.size(); i += 1 ) cout << arr1[i]->i << ',' << arr1[i]->j << ' ';
			cout << endl;
			uArrayPtrFill( S, arr2, 10, 2, 3 );
			for ( size_t i = 0; i < arr2.size(); i += 1 ) cout << arr2[i]->i << ',' << arr2[i]->j << ' ';
			cout << endl;
			throw E();
		} catch( E ) {}
		cout << endl;
	}
} // main
