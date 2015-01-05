//                              -*- Mode: C++ -*-
//
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// scan.c --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:11:49 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Jul 18 15:04:06 2014
// Update Count     : 67
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

#include "hash.h"
#include "token.h"
#include "table.h"
#include "key.h"
#include "scan.h"

#include <cstdio>					// EOF

//#define __U_DEBUG_H__

#ifdef __U_DEBUG_H__
#include <iostream>

using std::cerr;
using std::endl;
#endif // __U_DEBUG_H__


void scan() {
    ahead = ahead->next_parse_token();

  if ( ahead->value == EOF ) return;

    if ( ahead->hash->value != 0 ) {
	// if the value of the hash associated with the look ahead token is non zero, it must be a keyword.  simply make
	// the value of the token the value of the keyword.

	ahead->value = ahead->hash->value;
    } else if ( ahead->symbol != NULL ) {
	// symbol has already been looked up and the parser has backtracked to it again.
    } else if ( ahead->value == IDENTIFIER ) {
	// use the symbol to determine whether the identifier is a type or a variable.

#ifdef __U_DEBUG_H__
	cerr << "scan: token " << ahead << " (" << ahead->hash->text << ") focus " << focus << endl;
#endif // __U_DEBUG_H__
	if ( focus != NULL ) {			// scanning mode
	    ahead->symbol = focus->search_table( ahead->hash );
	    if ( ahead->symbol != NULL ) {
#ifdef __U_DEBUG_H__
		cerr << "scan: setting token " << ahead << " (" << ahead->hash->text << ") to value " << ahead->symbol->value << endl;
#endif // __U_DEBUG_H__
		ahead->value = ahead->symbol->value;
	    } // if
	} // if
    } // if
} // scan


void unscan( token_t *back ) {
    for ( token_t *t = back->next_parse_token(); t != ahead->next_parse_token(); t = t->next_parse_token() ) {
#ifdef __U_DEBUG_H__
	if ( t->value == TYPE || t->symbol != NULL ) cerr << "unscan: clearing token " << ahead << " (" << ahead->hash->text << ")" << endl;
#endif // __U_DEBUG_H__
	if ( t->value == TYPE ) t->value = IDENTIFIER;
	t->symbol = NULL;
    } // for
    ahead = back;
} // unscan


// Local Variables: //
// compile-command: "make install" //
// End: //
