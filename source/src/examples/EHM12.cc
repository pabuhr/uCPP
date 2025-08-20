//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2025
// 
// EHM12.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon May 26 23:10:43 2025
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue May 27 04:56:09 2025
// Update Count     : 2
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
using namespace std;

#include <uActor.h>

_Exception Logging {};

struct D {
    void raise() { _Resume Logging(); }					// match with d.Logging
} d;

struct E {
    void raise() { _Resume Logging(); }					// match with e.Logging
} e;

void h() {
    d.raise();
    cout << "after d.Logging" << endl;
    e.raise();
    cout << "after d.Logging" << endl;
    _Resume Logging();
}

void g() {
    try {
        h();
    } _CatchResume( d.Logging & logging ) {				// just catch "d"
        cout << "g d.Logging" << endl;
		void * prev = logging.setRaiseObject( (void *)&e ); // temporally change bound object to e
        _Resume; // re-resume exception e.Logging
        cout << "g after e.Logging" << endl;
		logging.setRaiseObject( prev );					// reset bound object back to previous value
        _Resume; // re-resume exception d.Logging
        cout << "g after d.Logging" << endl;
    }
}

void f() {
    try {
        g();
    } _CatchResume( d.Logging & ) {						// just catch "d"
        cout << "f d.Logging" << endl;
    } _CatchResume( e.Logging & ) {						// just catch "e"
        cout << "f e.Logging " << endl;
        _Resume; // re-resume exception e.Logging
        cout << "f after e.Logging " << endl;
    }
}

int main() {
	try {
		f();
	} _CatchResume ( Logging ) {						// catch any Logging
		cout << "Logging" << endl;
	} // try
}
