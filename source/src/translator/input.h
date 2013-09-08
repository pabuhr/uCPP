//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// input.h --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:38:14 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri May 13 22:09:47 2011
// Update Count     : 17
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


#ifndef __INPUT_H__
#define __INPUT_H__

void read_all_input();

#define EQ 256
#define NE 257
#define LE 258
#define GE 259

#define PLUS_ASSIGN 260
#define MINUS_ASSIGN 261
#define LSH_ASSIGN 262
#define RSH_ASSIGN 263
#define AND_ASSIGN 264
#define XOR_ASSIGN 265
#define OR_ASSIGN 266
#define MULTIPLY_ASSIGN 267
#define DIVIDE_ASSIGN 268
#define MODULUS_ASSIGN 269

#define AND_AND 270
#define OR_OR 271
#define PLUS_PLUS 272
#define MINUS_MINUS 273
#define LSH 274
#define RSH 275
#define GMIN 276
#define GMAX 277

#define ARROW 278

#define ARROW_STAR 279
#define DOT_STAR 280

#define CHARACTER 281
#define STRING 282
#define NUMBER 283

#define IDENTIFIER 284
#define LABEL 285
#define TYPE 286

#define DOT_DOT 287
#define DOT_DOT_DOT 288

#define COLON_COLON 289

#define ERROR 290
#define WARNING 291

#define CODE 292

#endif // __INPUT_H__

// Local Variables: //
// compile-command: "make install" //
// End: //
