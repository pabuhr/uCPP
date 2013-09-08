//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// symbol.c --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:18:22 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Jul  4 15:59:11 2011
// Update Count     : 40
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

#include <cstddef>					// NULL
#include "table.h"
#include "symbol.h"


symbol_data_t::symbol_data_t() {
    found = NULL;
    table = NULL;
    key = 0;
    index = DESTRUCTORPOSN;				// start allocating entry bits from this value
    base = NULL;
    used = false;
    base_token = NULL;
    left = right = NULL;
} // symbol_data_t::symbol_data_t


symbol_t::symbol_t( int v, hash_t *h ) {
    value = v;
    hash = h;
    copied = false;
    data = new symbol_data_t;
} // symbol_t::symbol_t


symbol_t::symbol_t( const symbol_t &other ) {
    value = other.value;
    hash = other.hash;
    copied = false;
    data = new symbol_data_t;
} // symbol_t::symbol_t


symbol_t::~symbol_t() {
    if ( ! copied ) {					// data shared for typedef and using
	delete data->table;
	delete data;
	data = NULL;
    } // if
    copied = false;
} // symbol_t::~symbol_t

// Local Variables: //
// compile-command: "make install" //
// End: //
