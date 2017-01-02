//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Richard Bilson and Ashif Harji 2003
// 
// uKernelThreads.h -- 
// 
// Author           : Richard Bilson and Ashif Harji
// Created On       : Wed Jul 16 16:44:10 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Oct  6 22:25:23 2016
// Update Count     : 49
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

#if defined( __linux__ ) || defined( __freebsd__ )

#if defined( __i386__ )
#include <cstddef>					// size_t

/* Read member of the thread descriptor directly.  */
#define THREAD_GETMEM(member) \
({										\
  __typeof__(((uKernelModule::uKernelModuleData *)0)->member) __value;		\
  /* There should not be any value with a size other than 1, 4 or 8.  */	\
  static_assert( sizeof(__value) == 1 || sizeof(__value) == 4 ||		\
		 sizeof(__value) == 8, "" );					\
  if (sizeof(__value) == 1)							\
    __asm__ __volatile__ ("movb %%gs:_ZN13uKernelModule17uKernelModuleBootE@ntpoff+%P2,%b0" \
			  : "=q" (__value)					\
			  : "0" (0),						\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
  else if (sizeof(__value) == 4)						\
    __asm__ __volatile__ ("movl %%gs:_ZN13uKernelModule17uKernelModuleBootE@ntpoff+%P1,%0" \
			  : "=r" (__value)					\
			  : "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
  else if (sizeof(__value) == 8)						\
    __asm__ __volatile__ ("movl %%gs:_ZN13uKernelModule17uKernelModuleBootE@ntpoff+%P1,%%eax\n\t" \
			  "movl %%gs:_ZN13uKernelModule17uKernelModuleBootE@ntpoff+%P2,%%edx" \
			  : "=A" (__value)					\
			  : "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)),				\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member) + 4));			\
  __value;									\
})

/* Set member of the thread descriptor directly.  */
#define THREAD_SETMEM(member, value) \
({										\
  __typeof__(((uKernelModule::uKernelModuleData *)0)->member) __value = (value); \
  /* There should not be any value with a size other than 1, 4 or 8.  */	\
  static_assert( sizeof(__value) == 1 || sizeof(__value) == 4 ||		\
		 sizeof(__value) == 8, "" );					\
  if (sizeof(__value) == 1)							\
    __asm__ __volatile__ ("movb %0,%%gs:_ZN13uKernelModule17uKernelModuleBootE@ntpoff+%P1" : \
			  : "q" (__value),					\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
  else if (sizeof(__value) == 4)						\
    __asm__ __volatile__ ("movl %0,%%gs:_ZN13uKernelModule17uKernelModuleBootE@ntpoff+%P1" : \
			  : "r" (__value),					\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
  else if (sizeof(__value) == 8)						\
    __asm__ __volatile__ ("movl %%eax,%%gs:_ZN13uKernelModule17uKernelModuleBootE@ntpoff+%P1\n\n" \
			  "movl %%edx,%%gs:_ZN13uKernelModule17uKernelModuleBootE@ntpoff+%P2" : \
			  : "A" (__value),					\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)),				\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member) + 4));			\
})

#elif defined( __x86_64__ )

/* Read member of the thread descriptor directly.  */
#define THREAD_GETMEM(member) \
({										\
  __typeof__(((uKernelModule::uKernelModuleData *)0)->member) __value;		\
  /* There should not be any value with a size other than 1, 4 or 8.  */	\
  static_assert( sizeof(__value) == 1 || sizeof(__value) == 4 ||		\
		 sizeof(__value) == 8, "" );					\
  if (sizeof(__value) == 1)							\
    __asm__ __volatile__ ("movb %%fs:_ZN13uKernelModule17uKernelModuleBootE@tpoff+%P2,%b0" \
			  : "=q" (__value)					\
			  : "0" (0),						\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
  else if (sizeof(__value) == 4)						\
    __asm__ __volatile__ ("movl %%fs:_ZN13uKernelModule17uKernelModuleBootE@tpoff+%P1,%0" \
			  : "=r" (__value)					\
			  : "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
  else if (sizeof(__value) == 8)						\
    __asm__ __volatile__ ("movq %%fs:_ZN13uKernelModule17uKernelModuleBootE@tpoff+%P1,%0" \
			  : "=r" (__value)					\
			  : "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
  __value;									\
})

/* Set member of the thread descriptor directly.  */
#define THREAD_SETMEM(member, value) \
({										\
  __typeof__(((uKernelModule::uKernelModuleData *)0)->member) __value = (value); \
  /* There should not be any value with a size other than 1, 4 or 8.  */	\
  static_assert( sizeof(__value) == 1 || sizeof(__value) == 4 ||		\
		 sizeof(__value) == 8, "" );					\
  if (sizeof(__value) == 1)							\
    __asm__ __volatile__ ("movb %0,%%fs:_ZN13uKernelModule17uKernelModuleBootE@tpoff+%P1" : \
			  : "q" (__value),					\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
  else if (sizeof(__value) == 4)						\
    __asm__ __volatile__ ("movl %0,%%fs:_ZN13uKernelModule17uKernelModuleBootE@tpoff+%P1" : \
			  : "r" (__value),					\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
  else if (sizeof(__value) == 8)						\
    __asm__ __volatile__ ("movq %0,%%fs:_ZN13uKernelModule17uKernelModuleBootE@tpoff+%P1" : \
			  : "r" (__value),					\
			    "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member)));				\
})

#elif defined( __ia64__ )

/* Making a system call is much like a procedure call in that it may clobber
   any of the scratch (caller-save) registers.  So we need to tell gcc that any
   of these registers can be clobbered when making a syscall from inline asm.
   
   Note that the 2.4 kernels don't seem to clobber scratch registers, so getting
   this wrong will not be apparent unless you try on 2.5 or later.  */

#define SCRATCH_REGS "r2", "r3", "r8", "r9", "r10", "r11", "r12", "r13", "r14",     \
                     "r15", "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", \
                     "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",        \
                     "b0", "b6", "b7",                                              \
                     "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",    \
                     "ar.ccv"

/* The ia64 architecture has no atomic offset load or store instructions; the
   offset computation must be done in one instruction, and the load or store
   instruction in another.  This can increase parallelism in the program, but
   if a signal occurs that changes the value of the base or the offset between
   the offset computation and the load or store, the load or store becomes
   invalid.  Since processor-local data is accessed at offsets from r13, and
   r13 may change as a result of a SIGALRM, a flag is set before attempting an
   offset load or store from r13; when this flag is set, the SIGALRM handler is
   deferred, i.e., control returns directly back to the point of interruption.

   The least-significant bit (lsb) of [r13] is used to store the flag.  On
   ia64, [r13] holds the DTV (dynamic task vector), which is a
   pointer-to-pointer and hence must be 8-byte aligned.  So the lsb would
   ordinarily be constant 0.  Between LOCK and UNLOCK the lsb is 1.

   One consequence of this approach is that some thread-local storage accesses
   fail if the access takes place while the flag is set.  It is unlikely but
   possible that errno could require such an access.

   Another consequence is that the lsb of [r13] must start at 0; ordinarily
   this is the case, but if for some reason it is not, signal processing fails.

   p7 is set before the flag is set.  If the SIGALRM handler is invoked and
   finds the flag set, it clears p7 to indicate that interrupt processing has
   been deferred.  This restricts the handling of multiple signal types, since
   if another signal handler is interrupted by the SIGALRM handler, p7 in the
   interrupted handler may be changed.
*/

#define DISABLE \
	";;cmp.eq p7, p0 = r0, r0\n"       \
	"fetchadd8.acq r14 = [r13], 1;;\n"

#define ENABLE \
	";;fetchadd8.rel r14 = [r13], -1\n"
	
/* If an interrupt is deferred within the LOCK/UNLOCK region, p7 is cleared.
   After the UNLOCK, the current processor is sent a SIGALRM to make up for the
   one that was deferred.

   It is unlikely but possible that the executing task is switched to a
   different processor between the UNLOCK and the point at which the SIGALRM is
   sent.  In this case, the SIGALRM sent by this code is unnecessary, since
   changing processors at this point could only be the result of handling
   another SIGALRM.  Hence, a spurious SIGALRM is sent to either the new
   processor or the old one, but this is not harmful.

   It would perhaps be cleaner (not to mention easier to debug) to have this
   code call a C handler function to send the signal.  However, this approach
   is impossible, due to a gcc bug (inline-asm/11563) precluding the call of a
   C function from inline assembly on ia64.  As a result, code to make the two
   system calls is written in assembler.
   
   It is important not to affect the value of errno in the user task.  Neither
   of these system calls can fail, and even if they could the fact that the are
   coded in assembler precludes an update to errno.
*/

/*
   Magic numbers: 1105 == __NR_gettid (<asm/unistd.h>)
                  1229 == __NR_tkill (<asm/unistd.h>)
                  10 == SIGUSR1 (<bits/signum.h>)
*/

#define HANDLE \
        "(p7) br.cond.sptk.many 0f;;\n" \
        "mov r15 = 1105\n"              \
        "break 0x100000;;\n"            \
        "mov r15 = 1229\n"              \
        "mov out0 = r8\n"               \
        "mov out1 = 10\n"               \
        "break 0x100000;;\n"            \
        "0:"

#define HANDLE_CLOBBERS "out0", "out1", SCRATCH_REGS

#define __LOAD_OFFSET \
        ";;mf\ncmp.eq p7, p0 = r0, r0\n"       \
        "mov r14 = @tprel(_ZN13uKernelModule17uKernelModuleBootE#)+%1\n"	\
	"fetchadd8.acq r15 = [r13], 1;;\n"

#define __ENABLE \
	"fetchadd8.rel r15 = [r13], -1\n"

#define GETMEM_ASM(operand, member) \
    __asm__ __volatile__ (__LOAD_OFFSET						\
        		  "add r14 = r13, r14;;\n"				\
                          operand " %0 = [r14];;\n"				\
                          __ENABLE						\
                          HANDLE						\
			  : "=r" (__value)			      		\
			  : "i" (offsetof (uKernelModule::uKernelModuleData, member)) \
			  : "r14", "r15", "p7", HANDLE_CLOBBERS )

#define THREAD_GETMEM(member) \
({										\
  __typeof__(((uKernelModule::uKernelModuleData *)0)->member) __value;		\
  /* There should not be any value with a size other than 1, 4 or 8. */		\
  static_assert( sizeof(__value) == 1 || sizeof(__value) == 4 ||		\
		 sizeof(__value) == 8, "" );					\
  if (sizeof (__value) == 1)							\
    GETMEM_ASM( "ld1", member );						\
  else if (sizeof (__value) == 4)						\
    GETMEM_ASM( "ld4", member );						\
  else if (sizeof (__value) == 8)						\
    GETMEM_ASM( "ld8", member );						\
  __value;									\
})

#define SETMEM_ASM(operand, member) \
    __asm__ __volatile__ (__LOAD_OFFSET						\
        		  "add r14 = r13, r14;;\n"				\
                          operand " [r14] = %0\n"				\
                          ENABLE						\
                          HANDLE						\
			  :					     		\
			  : "r" (__value),			        	\
			    "i" (offsetof (uKernelModule::uKernelModuleData, member)) \
			  : "r14", "r15", "p7", HANDLE_CLOBBERS )

/* Set member of the thread descriptor directly.  */
#define THREAD_SETMEM(member, value) \
({										\
  __typeof__(((uKernelModule::uKernelModuleData *)0)->member) __value = (value); \
  /* There should not be any value with a size other than 1, 4 or 8. */		\
  static_assert( sizeof(__value) == 1 || sizeof(__value) == 4 ||		\
		 sizeof(__value) == 8, "" );					\
  if (sizeof (__value) == 1)							\
    SETMEM_ASM( "st1", member );						\
  else if (sizeof (__value) == 4)						\
    SETMEM_ASM( "st4", member );						\
  else if (sizeof (__value) == 8)						\
    SETMEM_ASM( "st8", member );						\
})

#else

    #error uC++ : internal error, unsupported architecture

#endif // hardware architectures


#elif defined( __solaris__ )

#if defined( __sparc__ )

// This assembler directive is necessary so the assembler knows a global register is in use.
// http://developers.sun.com/solaris/articles/sparcv9abi.html
//register unsigned long sparc_tp __asm__( "%g7" );
__asm__( ".register %g7,#ignore" );

#define THREAD_GETMEM(member) \
({										\
  __typeof__(((uKernelModule::uKernelModuleData *)0)->member) __value;		\
  /* There should not be any value with a size other than 1, 4 or 8.  */	\
  static_assert( sizeof(__value) == 1 || sizeof(__value) == 4 ||		\
		 sizeof(__value) == 8, "" );					\
  if (sizeof(__value) == 1)							\
    __asm__ __volatile__ ("sethi %%tle_hix22(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
		  	  "xor %%g1, %%tle_lox10(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
			  "add %%g1, %1, %%g1\n\t"				\
			  "ldub [%%g7+%%g1], %0"				\
			  : "=r" (__value)					\
			  : "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member))				\
			  : "%g1" );						\
  else if (sizeof(__value) == 4)						\
    __asm__ __volatile__ ("sethi %%tle_hix22(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
		  	  "xor %%g1, %%tle_lox10(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
			  "add %%g1, %1, %%g1\n\t"				\
			  "lduw [%%g7+%%g1], %0"				\
			  : "=r" (__value)					\
			  : "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member))				\
			  : "%g1" );						\
  else if (sizeof(__value) == 8)						\
    __asm__ __volatile__ ("sethi %%tle_hix22(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
		  	  "xor %%g1, %%tle_lox10(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
			  "add %%g1, %1, %%g1\n\t"				\
			  "ldx [%%g7+%%g1], %0"					\
			  : "=r" (__value)					\
			  : "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member))				\
			  : "%g1" );						\
  __value;									\
})

/* Set member of the thread descriptor directly.  */
#define THREAD_SETMEM(member, value) \
({										\
  __typeof__(((uKernelModule::uKernelModuleData *)0)->member) __value = (value); \
  /* There should not be any value with a size other than 1, 4 or 8.  */	\
  static_assert( sizeof(__value) == 1 || sizeof(__value) == 4 ||		\
		 sizeof(__value) == 8, "" );					\
  if (sizeof(__value) == 1)							\
    __asm__ __volatile__ ("sethi %%tle_hix22(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
		  	  "xor %%g1, %%tle_lox10(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
			  "add %%g1, %1, %%g1\n\t"				\
			  "stb %0, [%%g7+%%g1]"					\
			  :: "r" (__value),					\
			     "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member))				\
			  : "%g1" );						\
  else if (sizeof(__value) == 4)						\
    __asm__ __volatile__ ("sethi %%tle_hix22(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
		  	  "xor %%g1, %%tle_lox10(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
			  "add %%g1, %1, %%g1\n\t"				\
			  "stw %0, [%%g7+%%g1]"					\
			  :: "r" (__value),					\
			     "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member))				\
			  : "%g1" );						\
  else if (sizeof(__value) == 8)						\
    __asm__ __volatile__ ("sethi %%tle_hix22(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
		  	  "xor %%g1, %%tle_lox10(_ZN13uKernelModule17uKernelModuleBootE), %%g1\n\t" \
			  "add %%g1, %1, %%g1\n\t"				\
			  "stx %0, [%%g7+%%g1]"					\
			  :: "r" (__value),					\
			     "i" (offsetof (uKernelModule::uKernelModuleData,	\
					   member))				\
			  : "%g1" );						\
})


#else

    #error uC++ : internal error, unsupported architecture

#endif // hardware architectures

#else

    #error uC++ : internal error, unsupported operating system

#endif // operating systems

#else  // uniprocessor kluge

#define THREAD_GETMEM(member) uKernelModule::uKernelModuleBoot.member
#define THREAD_SETMEM(member, value) uKernelModule::uKernelModuleBoot.member = (value)

#endif // __U_MULTI__
