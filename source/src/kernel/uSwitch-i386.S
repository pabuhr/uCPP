//                               -*- Mode: Asm -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uSwitch-i386.S -- context switch for x86
// 
// Author           : Peter A. Buhr
// Created On       : Sun Oct  9 11:46:38 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Jun 29 13:54:26 2011
// Update Count     : 26
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

#define PTR_BYTE	4
#define SP_OFFSET	( 0 * PTR_BYTE )
#define FP_OFFSET	( 1 * PTR_BYTE )
#define PC_OFFSET	( 2 * PTR_BYTE )

.text
	.align 2
.globl	uSwitch
uSwitch:

	// Copy the "from" context argument from the stack to register eax
	// Return address is at 0(%esp), with parameters following
	
	movl 4(%esp),%eax

	// Save volatile registers on the stack.

	pushl %ebx
	pushl %edi
	pushl %esi

	// Save old context in the "from" area.

	movl %esp,SP_OFFSET(%eax)
	movl %ebp,FP_OFFSET(%eax)
//	movl 4(%ebp),%ebx	// save previous eip for debugger
//	movl %ebx,PC_OFFSET(%eax)

	// Copy the "to" context argument from the stack to register eax
	// Having pushed three words (= 12 bytes) on the stack, the
	// argument is now at 8 + 12 = 20(%esp)

	movl 20(%esp),%eax

	// Load new context from the "to" area.

	movl SP_OFFSET(%eax),%esp
	movl FP_OFFSET(%eax),%ebp

	// Load volatile registers from the stack.

	popl %esi
	popl %edi
	popl %ebx

	// Return to thread.

	ret

// Local Variables: //
// compile-command: "make install" //
// End: //
