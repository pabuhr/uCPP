//                              -*- Mode: C++ -*-
// 
// uC++ Version 7.0.0, Copyright (C) Martin Karsten 1995
// 
// ArchBasics.h -- 
// 
// Author           : Martin Karsten
// Created On       : Sat May 13 14:54:59 1995
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Oct 22 14:18:38 2011
// Update Count     : 14
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

#ifndef _ArchBasics_h_
#define _ArchBasics_h_ 1

#if defined( __sparc__ ) && defined( __solaris__ ) /* Solaris 2.x */

#include <sys/procfs.h>

typedef long*			CodeAddress;
typedef long			Instruction;
typedef	long*			DataAddress;
typedef long			DataField;
typedef	char*			InternalAddress;
typedef unsigned long	CalcAddress;
typedef	prgreg_t		Register;

// this is the minimal set of registers needed to reconstruct a frame backtrace
struct MinimalRegisterSet {
	Register	sp;
	Register	fp;
	Register	pc;
}; // struct MinimalRegisterSet

// I don't want to use a function call in any case, hence this is a macro
#define CREATE_MINIMAL_REGISTER_SET(regs)\
	asm(" st %%sp,%0" : "=m" (regs.sp) : );\
	asm(" st %%fp,%0" : "=m" (regs.fp) : );\
	asm(" st %%i7,%0" : "=m" (regs.pc) : );

#elif defined( __sparc__ )	/* SunOS 4.1 */

#include <machine/reg.h>

typedef long*			CodeAddress;
typedef long			Instruction;
typedef	long*			DataAddress;
typedef long			DataField;
typedef	char*			InternalAddress;
typedef unsigned long	CalcAddress;
typedef	int				Register;

// This is kind of a hack. Usually only 19 registers are supported,
// but I just extend it to also store the FP register at the last position
typedef int				prgregset_t[20];
typedef int				prgreg_t;

#define R_SP    O6
#define R_PC    PC
#define R_nPC   nPC
#define R_Y     Y
#define R_FP	19

// this is the minimal set of registers needed to reconstruct a frame backtrace
struct MinimalRegisterSet {
	Register	sp;
	Register	fp;
	Register	pc;
}; // struct MinimalRegisterSet

// I don't want to use a function call in any case, hence this is a macro
#define CREATE_MINIMAL_REGISTER_SET(regs)\
	asm(" st %%sp,%0" : "=m" (regs.sp) : );\
	asm(" st %%fp,%0" : "=m" (regs.fp) : );\
	asm(" st %%i7,%0" : "=m" (regs.pc) : );

#elif defined( __i386__ )
#include <sys/ptrace.h>
#define NUM_REGS 16			// from gdb/config/i386/tm-i386v.h
typedef int				prgregset_t[NUM_REGS+1]; // to hold ORIG_EAX
typedef int				prgreg_t;
#define R_SP  UESP				/* Contains address of top of stack */
#define R_PC  EIP
#define R_FP  EBP				/* Contains address of executing stack frame */

typedef long*		CodeAddress;
typedef char		Instruction;
typedef long*		DataAddress;
typedef long		DataField;
typedef int			Register;
typedef char*		InternalAddress;

struct MinimalRegisterSet {
	Register	sp;
	Register	fp;
	Register	pc;
}; // struct MinimalRegisterSet

#define CREATE_MINIMAL_REGISTER_SET(regs)\
	asm(" movl %%esp,%0" : "=m" (regs.sp) : );\
	asm(" movl %%ebp,%0" : "=m" (regs.fp) : );\
	asm(" movl 4(%ebp),%eax");\
	asm(" movl %%eax,%0" : "=m" (regs.pc) : );

#else /* any not supported architecture */

typedef long*		CodeAddress;
typedef long		Instruction;
typedef long*		DataAddress;
typedef long		DataField;
typedef int			Register;
typedef char*		InternalAddress;

struct MinimalRegisterSet {
	Register	sp;
	Register	fp;
	Register	pc;
}; // struct MinimalRegisterSet

#define CREATE_MINIMAL_REGISTER_SET(regs)\
	regs.sp = 0;\
	regs.fp = 0;\
	regs.pc = 0;

#endif // __sparc__ && __solaris__

// once for all
#include <string.h>

// same for all UNIXs ?
typedef	uPid_t			OSKernelThreadId;

#endif // _ArchBasics_h_


// Local Variables: //
// tab-width: 4 //
// End: //
