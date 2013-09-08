//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// main.h --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 16:06:46 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri May 13 22:05:48 2011
// Update Count     : 27
//

#ifndef __MAIN_H__
#define __MAIN_H__

#include <iostream>

using std::istream;
using std::ostream;

extern istream *yyin;
extern ostream *yyout;

extern bool error;
extern bool Yield;					// name "yield" already taken
extern bool verify;
extern bool trace;
extern bool profile;
extern bool gnu;
extern bool user;

int main( int argc, char *argv[] );

#endif // __MAIN_H__

// Local Variables: //
// compile-command: "make install" //
// End: //
