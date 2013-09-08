//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// output.h --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:41:53 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri May 13 22:13:33 2011
// Update Count     : 13
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

#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <token.h>

extern char *file;
extern token_t *file_token;
extern unsigned int line;
void parse_directive( char *text, char *&file, unsigned int &line );

void write_all_output();

#endif // __OUTPUT_H__

// Local Variables: //
// compile-command: "make install" //
// End: //
