//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Richard Bilson and Ashif Harji 2003
// 
// uKernelThreads.h -- 
// 
// Author           : Richard Bilson and Ashif Harji
// Created On       : Wed Jul 16 16:44:10 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Apr 12 17:35:37 2019
// Update Count     : 63
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

// Most of this code is borrowed from the gnu glibc library machine dependent pthread functions.  This code has been
// modified from the original source around May 2003.


// Note, must use "asm" for THREAD_GETMEM/THREAD_SETMEM versus direct access, as in:
//
//  #define THREAD_GETMEM(member) uKernelModule::uKernelModuleBoot.member
//  #define THREAD_SETMEM(member, value) uKernelModule::uKernelModuleBoot.member = (value)
//
// To do this correctly, the compiler must reload the uKernelModuleBoot pointer after a context switch as if it is
// volatile. But it often optimizes away the reload assuming the (kernel) thread does not change during the execution of
// a routine, which is a reasonable sequential assumption. Unfortunately, the TSL pointer inside the __thread access
// cannot be explicitly marked as volatile because the pointer is implicit.


#if defined(__U_MULTI__)

#if defined( __i386__ )

#define THREAD_GETMEM( member ) uKernelModule::uKernelModuleBoot.member
#define THREAD_SETMEM( member, value ) uKernelModule::uKernelModuleBoot.member = value;

#elif defined( __x86_64__ )

#define THREAD_GETMEM( member ) uKernelModule::uKernelModuleBoot.member
#define THREAD_SETMEM( member, value ) uKernelModule::uKernelModuleBoot.member = value;

#endif // hardware architectures

#else  // uniprocessor kluge

#define THREAD_GETMEM(member) uKernelModule::uKernelModuleBoot.member
#define THREAD_SETMEM(member, value) uKernelModule::uKernelModuleBoot.member = (value)

#endif // __U_MULTI__
