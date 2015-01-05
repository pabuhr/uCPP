//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 2003
// 
// EHM9.cc -- recursive resumption
// 
// Author           : Peter A. Buhr
// Created On       : Sun Dec  7 12:15:30 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Aug 14 09:15:13 2009
// Update Count     : 35
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Event R1 {};
_Event R2 {};

// Checks marking of handlers is performed correctly during propagation.
// Marking prevents recursive resumption.

void uMain::main () {
	try {
		try {
			try {
				osacquire( cout ) << "before raise" << endl;
				_Resume R1();
				osacquire( cout ) << "after raise" << endl;
			} _CatchResume( R2 ) {
				osacquire( cout ) << "enter H3" << endl;
				_Resume R1();
				osacquire( cout ) << "exit  H3" << endl;
			} // try
		} _CatchResume( R1 ) {
			osacquire( cout ) << "enter H2" << endl;
			_Resume R2();
			osacquire( cout ) << "exit  H2" << endl;
		} // try
	} _CatchResume( R2 ) {
		osacquire( cout ) << "enter H1" << endl;
		osacquire( cout ) << "exit  H1" << endl;
	} // try
	osacquire( cout ) << "finished" << endl;
} // uMain::main


// Local Variables: //
// tab-width: 4 //
// End: //
