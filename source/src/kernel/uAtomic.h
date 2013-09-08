//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Richard C. Bilson 2004
// 
// uAtomic.h -- atomic routines for various processors
// 
// Author           : Richard C. Bilson
// Created On       : Thu Sep 16 13:57:26 2004
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Jun 10 17:52:27 2011
// Update Count     : 71
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

#if defined( __ia64__ )
#include <ia64intrin.h>
#endif // __ia64__


#if defined( __i386__ ) || defined( __x86_64__ )
static inline unsigned long long int uRead_tsc() {
    unsigned long long int result;
#if defined( __x86_64__ )
    asm volatile ( "rdtsc\n"
		   "salq $32, %%rdx\n"
		   "orq %%rdx, %%rax"
		   : "=a" (result) : : "rdx"
	);
#elif defined( __i386__ )
    asm volatile ( "rdtsc" : "=A" (result) );
#else
    #error uC++ : internal error, unsupported architecture
#endif
    return result;
} // uRead_tsc
#endif


inline int uTestSet( unsigned int &lock ) {
#if defined( GLIBCXX_ENABLE_ATOMIC_BUILTINS ) || defined( __ia64__ ) || defined( __sparc__ )
    return __sync_lock_test_and_set( &lock, 1 );
#elif defined( __i386__ ) || defined( __x86_64__ )
#if defined( __GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ > 2 || __GNUC__ == 4 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ >= 1)
    return __sync_lock_test_and_set( &lock, 1 );
#else
    int result = 1;
    asm( "xchgl %0,%1" : "+q" (result), "+m" (lock) );
    return result;
#endif
#else
    #error uC++ : internal error, unsupported architecture
#endif
} // uTestSet


inline void uTestReset( unsigned int &lock ) {
#if defined( GLIBCXX_ENABLE_ATOMIC_BUILTINS ) || defined( __ia64__ ) || defined( __sparc__ )
    __sync_lock_release( &lock );
#elif defined( __i386__ ) || defined( __x86_64__ )
#if defined( __GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ > 2 || __GNUC__ == 4 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ >= 1)
    __sync_lock_release( &lock );
#else
    lock = 0;					// unlock
#endif
#else
    #error uC++ : internal error, unsupported architecture
#endif
} // uTestReset


template< typename T > inline bool uCompareAssign( volatile T &loc, T comp, T replacement ) {
#if defined( GLIBCXX_ENABLE_ATOMIC_BUILTINS ) || defined( __ia64__ )
#ifdef __sync_bool_compare_and_swap			// broken macro, replace it
#if __U_WORDSIZE__ == 32
    return __sync_bool_compare_and_swap_si((int *)(void *)(&loc),(int)(comp),(int)(replacement));
#else
    return __sync_bool_compare_and_swap_di((long *)(void *)(&loc),(long)(comp),(long)(replacement));
#endif // __U_WORDSIZE__ == 32
#else
    return __sync_bool_compare_and_swap( &loc, comp, replacement );
#endif // __sync_bool_compare_and_swap
#elif defined( __sparc__ )
    return __sync_bool_compare_and_swap( &loc, comp, replacement );
#elif defined( __i386__ ) || defined( __x86_64__ )
#if defined( __GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ > 2 || __GNUC__ == 4 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ >= 1)
    return __sync_bool_compare_and_swap( &loc, comp, replacement );
#else
    unsigned char ret;
    asm volatile (
#ifdef __U_MULTI__
	"lock "
#endif // ! __U_MULTI__
	"cmpxchg %3, %0\n\t"
	"sete %2" : "+m" (loc), "+a" (comp), "=q" (ret) : "r" (replacement) );
    return ret;
#endif
#else
    #error uC++ : internal error, unsupported architecture
#endif
} // uCompareAssign


template< typename T > inline T uFetchAdd( volatile T &counter, int amt ) {
#if defined( GLIBCXX_ENABLE_ATOMIC_BUILTINS ) || defined( __ia64__ ) || defined( __sparc__ )
    return __sync_fetch_and_add( &counter, amt );
#elif defined( __i386__ ) || defined( __x86_64__ )
#if defined( __GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ > 2 || __GNUC__ == 4 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ >= 1)
    return __sync_fetch_and_add( &counter, amt );
#else
    asm volatile (
#ifdef __U_MULTI__
	"lock "
#endif // ! __U_MULTI__
	"xaddl %0,%1" : "+r" (amt), "+m" (counter) );
    return amt;
#endif
#else
    #error uC++ : internal error, unsupported architecture
#endif
} // uFetchAdd


// Local Variables: //
// compile-command: "make install" //
// End: //
