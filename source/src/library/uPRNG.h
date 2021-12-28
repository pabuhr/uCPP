//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2021
// 
// uPRNG.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sat Dec 25 17:48:48 2021
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Dec 27 23:43:11 2021
// Update Count     : 11
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


#pragma once

#include <cstdint>										// uint32_t

// Sequential Pseudo Random-Number Generator : generate repeatable sequence of values that appear random.
//
// Declaration :
//   PRNG sprng( 1009 ) - set starting seed versus random seed
//   
// Interface :
//   sprng.set_seed( 1009 ) - set starting seed for ALL kernel threads versus random seed
//   sprng.get_seed() - read seed
//   sprng() - generate random value in range [0,UINT_MAX]
//   sprng( u ) - generate random value in range [0,u)
//   sprng( l, u ) - generate random value in range [l,u]
//   sprng.calls() - number of generated random value so far
//
// Examples : generate random number between 5-21
//   sprng() % 17 + 5;	values 0-16 + 5 = 5-21
//   sprng( 16 + 1 ) + 5;
//   sprng( 5, 21 );
//   sprng.calls();

class PRNG {
	uint32_t PRNGcnt = 0;
	uint32_t seed;										// current seed
	uint32_t state;										// random state
  public:
	PRNG() { set_seed( uRdtsc() ); }					// random seed
	PRNG( uint32_t seed ) { set_seed( seed ); }			// fixed seed
	void set_seed( uint32_t seed_ ) { state = seed = seed_; } // set seed
	uint32_t get_seed() const __attribute__(( warn_unused_result )) { return seed; } // get seed
	uint32_t operator()() __attribute__(( warn_unused_result )); // [0,UINT_MAX]
	uint32_t operator()( uint32_t u ) __attribute__(( warn_unused_result )) { return operator()() % u; } // [0,u)
	uint32_t operator()( uint32_t l, uint32_t u ) __attribute__(( warn_unused_result )) { return operator()( u - l + 1 ) + l; } // [l,u]
	uint32_t calls() const __attribute__(( warn_unused_result )) { return PRNGcnt; }
}; // PRNG

//===============================================================

// Concurrent per Kernel-Thread Pseudo Random-Number Generator : generate repeatable sequence of values that appear random.
//
// Interface :
//   set_seed( 1009 ) - fixed seed for all kernel threads versus random seed
//   get_seed() - read seed
//   prng() - generate random value in range [0,UINT_MAX]
//   prng( u ) - generate random value in range [0,u)
//   prng( l, u ) - generate random value in range [l,u]
//
// Examples : generate random number between 5-21
//   prng() % 17 + 5;	values 0-16 + 5 = 5-21
//   prng( 16 + 1 ) + 5;
//   prng( 5, 21 );

extern void set_seed( uint32_t seed );					// set per thread seed
extern uint32_t get_seed();								// get seed
extern uint32_t prng() __attribute__(( warn_unused_result )); // [0,UINT_MAX]
static inline uint32_t prng( uint32_t u ) __attribute__(( warn_unused_result ));
static inline uint32_t prng( uint32_t u ) { return prng() % u; } // [0,u)
static inline uint32_t prng( uint32_t l, uint32_t u ) __attribute__(( warn_unused_result ));
static inline uint32_t prng( uint32_t l, uint32_t u ) { return prng( u - l + 1 ) + l; } // [l,u]


// Local Variables: //
// compile-command: "make install" //
// End: //
