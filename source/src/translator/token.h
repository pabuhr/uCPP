//                              -*- Mode: C++ -*-
//
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// token.h --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:48:16 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Jan 12 21:57:00 2016
// Update Count     : 54
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

#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <cstddef>					// size_t

class hash_t;
struct symbol_t;

struct token_t {
    token_t *fore;					// next token
    token_t *aft;					// previous token
    int value;						// lexer code for token
    hash_t *hash;					// hashed token entry
    symbol_t *symbol;					// symbol table for identifiers
    token_t *left;					// start of base_specifier
    token_t *right;					// end of base_specifier

    void *operator new( size_t );
    void operator delete( void *p );
    void init() { symbol = nullptr; left = right = nullptr; }
    token_t() { hash = nullptr; init(); }
    token_t( const token_t &t ) { *this = t; }
    token_t( int value, hash_t *hash ) : value( value ), hash( hash ) {	init(); }
    ~token_t();
    void add_token_after( token_t &before );
    void add_token_before( token_t &after );
    void remove_token();
    token_t *next_parse_token();
    token_t *prev_parse_token();
}; // token_t


extern token_t *ahead;					// current token being parsed


class token_list_t {
  private:
    token_t head;
    token_t tail;
  public:
    token_list_t();
    ~token_list_t();
    void add_to_head( token_t &insert );
    void add_to_tail( token_t &insert );
    token_t *remove_from_head();
    token_t *remove_from_tail();
    token_t *get_head();
    token_t *get_tail();
    int empty();
}; // token_list_t


extern token_list_t *token_list;


#endif // __TOKEN_H__

// Local Variables: //
// compile-command: "make install" //
// End: //
