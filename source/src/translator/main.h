//                              -*- Mode: C++ -*-
//
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// main.h --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 16:06:46 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Jul 15 16:28:55 2014
// Update Count     : 29
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
extern bool profile;
extern bool stdcpp11;
extern bool user;

int main( int argc, char *argv[] );

#endif // __MAIN_H__

// Local Variables: //
// compile-command: "make install" //
// End: //
