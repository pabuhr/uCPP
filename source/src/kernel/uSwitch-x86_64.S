//                               -*- Mode: Asm -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// uSwitch-x64.S -- context switch for x86_64
// 
// Author           : Richard C. Bilson
// Created On       : Mon Feb 28 13:36:42 2005
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Jul 13 22:26:57 2012
// Update Count     : 3
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

// This context switch routine depends on the fact that the stack of a new
// thread has been set up to look like the thread has saved its context in
// the normal manner.
//
// void uSwitch( uMachContext *from, uMachContext *to );

// Offsets in the context structure. This needs to be synchronized with the
// high level code a little better.

#define PTR_BYTE	8
#define SP_OFFSET	( 0 * PTR_BYTE )
#define FP_OFFSET	( 1 * PTR_BYTE )
#define PC_OFFSET	( 2 * PTR_BYTE )

.text
	.align 2
.globl	uSwitch
uSwitch:

	// Save volatile registers on the stack.

	pushq %r15
	pushq %r14
	pushq %r13
	pushq %r12
	pushq %rbx

	// Save old context in the "from" area.

	movq %rsp,SP_OFFSET(%rdi)
	movq %rbp,FP_OFFSET(%rdi)

	// Load new context from the "to" area.

	movq SP_OFFSET(%rsi),%rsp
	movq FP_OFFSET(%rsi),%rbp

	// Load volatile registers from the stack.

	popq %rbx
	popq %r12
	popq %r13
	popq %r14
	popq %r15

	// Return to thread.

	ret

.text
	.align 2
.globl	uInvokeStub
uInvokeStub:
	movq %rbx, %rdi
	jmp *%r12

// Local Variables: //
// compile-command: "make install" //
// End: //
