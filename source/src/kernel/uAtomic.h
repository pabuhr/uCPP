//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Richard C. Bilson 2004
// 
// uAtomic.h -- atomic routines for various processors
// 
// Author           : Richard C. Bilson
// Created On       : Thu Sep 16 13:57:26 2004
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed May  3 18:05:39 2023
// Update Count     : 164
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

static inline __attribute__((always_inline)) unsigned long long int uRdtsc() {
	unsigned long long int result;
#if defined( __x86_64__ )
	asm volatile ( "rdtsc\n"
		   "salq $32, %%rdx\n"
		   "orq %%rdx, %%rax"
		   : "=a" (result) : : "rdx"
	);
#elif defined( __i386__ )
	asm volatile ( "rdtsc" : "=A" (result) );
#elif defined( __ARM_ARCH )
	// https://github.com/google/benchmark/blob/v1.1.0/src/cycleclock.h#L116
	asm volatile ( "mrs %0, cntvct_el0" : "=r" (result) );
	return result;
#else
	#error unsupported architecture
#endif
	return result;
} // uRdtsc


static inline __attribute__((always_inline)) void uFence() { // fence to prevent code movement
#if defined(__x86_64)
	//#define Fence() __asm__ __volatile__ ( "mfence" )
	__asm__ __volatile__ ( "lock; addq $0,(%%rsp);" ::: "cc" );
#elif defined(__i386)
	__asm__ __volatile__ ( "lock; addl $0,(%%esp);" ::: "cc" );
#elif defined( __ARM_ARCH )
	__asm__ __volatile__ ( "DMB ISH" ::: );
#else
	#error unsupported architecture
#endif
} // uFence


static inline __attribute__((always_inline)) void uPause() { // pause to prevent excess processor bus usage
#if defined( __i386 ) || defined( __x86_64 )
	__asm__ __volatile__ ( "pause" ::: );
#elif defined( __ARM_ARCH )
	__asm__ __volatile__ ( "YIELD" ::: );
#else
	#error unsupported architecture
#endif
} // uPause


template< typename T > static inline __attribute__((always_inline)) bool uTestSet( volatile T & lock ) {
	//return __sync_lock_test_and_set( &lock, 1 );
	return __atomic_test_and_set( &lock, __ATOMIC_ACQUIRE );
} // uTestSet

template< typename T > static inline __attribute__((always_inline)) void uTestReset( volatile T & lock ) {
	//__sync_lock_release( &lock );
	__atomic_clear( &lock, __ATOMIC_RELEASE );
} // uTestReset


template< typename T > static inline __attribute__((always_inline)) T uFetchAssign( volatile T & loc, T replacement ) {
	//return __sync_lock_test_and_set( &loc, replacement );
	return __atomic_exchange_n( &loc, replacement, __ATOMIC_ACQUIRE );
} // uFetchAssign


template< typename T > static inline __attribute__((always_inline)) T uFetchAdd( volatile T & counter, int increment ) {
	//return __sync_fetch_and_add( &counter, increment );
	return __atomic_fetch_add( &counter, increment, __ATOMIC_SEQ_CST );
} // uFetchAdd

template< typename T > static inline __attribute__((always_inline)) T uFetchAdd( volatile T & counter, int increment, int order ) {
	//return __sync_fetch_and_add( &counter, increment );
	return __atomic_fetch_add( &counter, increment, order );
} // uFetchAdd


template< typename T > static inline __attribute__((always_inline)) bool uCompareAssign( volatile T & loc, T comp, T replacement ) {
	//return __sync_bool_compare_and_swap( &loc, comp, replacement );
	return __atomic_compare_exchange_n( &loc, &comp, replacement, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
} // uCompareAssign


template< typename T > static inline __attribute__((always_inline)) bool uCompareAssignValue( volatile T & loc, T & comp, T replacement ) {
	return __atomic_compare_exchange_n( &loc, &comp, replacement, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
} // uCompareAssignValue


// Local Variables: //
// compile-command: "make install" //
// End: //
