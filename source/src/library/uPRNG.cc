//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2021
// 
// uPRNG.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sat Dec 25 17:50:36 2021
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Dec 27 14:29:54 2021
// Update Count     : 35
// 
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


#define __U_KERNEL__
#include <uC++.h>
#include <uPRNG.h>

static uint32_t seed = 0;								// current seed
static thread_local uint32_t state;						// random state

void set_seed( uint32_t seed_ ) { state = seed = seed_; }
uint32_t get_seed() { return seed; }

#define GENERATOR LCG

//=========================================================

uint32_t MarsagliaXor( uint32_t & state ) {
	if ( UNLIKELY( seed == 0 ) ) set_seed( uRdtsc() );
	else if ( UNLIKELY( state == 0 ) ) state = seed;
	state ^= state << 6;
	state ^= state >> 21;
	state ^= state << 7;
	return state;
} // MarsagliaXor

//=========================================================

uint32_t LCG( uint32_t & state ) {						// linear congruential generator
	if ( UNLIKELY( seed == 0 ) ) set_seed( uRdtsc() );
	else if ( UNLIKELY( state == 0 ) ) state = seed;
	return state = 36973 * (state & 65535) + (state >> 16);
} // LCG

//=========================================================

uint32_t PRNG::operator()() { PRNGcnt += 1; return GENERATOR( state ); }

uint32_t prng() { return GENERATOR( state ); }


// Local Variables: //
// compile-command: "make install" //
// End: //
