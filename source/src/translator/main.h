//                              -*- Mode: C++ -*-
//
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// main.h --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 16:06:46 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Jan 21 09:17:18 2019
// Update Count     : 31
//


#pragma once


#include <iostream>

using std::istream;
using std::ostream;

extern istream *yyin;
extern ostream *yyout;

extern bool error;
extern bool profile;
extern bool stdcpp11;
extern bool user;

int main( int argc, char *argv[] );



// Local Variables: //
// compile-command: "make install" //
// End: //
