//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// key.c --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:06:35 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Feb 20 14:10:40 2013
// Update Count     : 103
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

#include "key.h"
#include "input.h"

#include <cstddef>					// NULL

keyword_t key[] = {
    { "asm", ASM },
    { "__asm", ASM },					// gcc specific
    { "__asm__", ASM },					// gcc specific
    { "_Atomic", ATOMIC },				// C11
    { "__attribute__", ATTRIBUTE },			// gcc specific
    { "auto", AUTO },
    { "bool", BOOL },
    { "break", BREAK },
    { "case", CASE },
    { "catch", CATCH },
    { "char", CHAR },
    { "class", CLASS },
    { "__complex__", COMPLEX },				// gcc specific
    { "const", CONST },
    { "__const", CONST },				// gcc specific
    { "__const__", CONST },				// gcc specific
    { "const_cast", CONST_CAST },
    { "continue", CONTINUE },
    { "default", DEFAULT },
    { "delete", DELETE },
    { "do", DO },
    { "double", DOUBLE },
    { "dynamic_cast", DYNAMIC_CAST },
    { "else", ELSE },
    { "enum", ENUM },
    { "explicit", EXPLICIT },
    { "export", EXPORT },
    { "__extension__", EXTENSION },			// gcc specific
    { "extern", EXTERN },
    { "false", FALSE },
    { "float", FLOAT },
    { "for", FOR },
    { "friend", FRIEND },
    { "goto", GOTO },
    { "if", IF },
    { "inline", INLINE },
    { "__inline", INLINE },				// gcc specific
    { "__inline__", INLINE },				// gcc specific
    { "int", INT },
    { "long", LONG },
    { "mutable", MUTABLE },
    { "namespace", NAMESPACE },
    { "new", NEW },
    { "operator", OPERATOR },
    { "private", PRIVATE },
    { "protected", PROTECTED },
    { "public", PUBLIC },
    { "register", REGISTER },
    { "reinterpret_cast", REINTERPRET_CAST },
    { "restrict", RESTRICT },
    { "__restrict", RESTRICT },
    { "__restrict__", RESTRICT },
    { "return", RETURN },
    { "short", SHORT },
    { "signed", SIGNED },
    { "__signed", SIGNED },				// gcc specific
    { "__signed__", SIGNED },				// gcc specific
    { "sizeof", SIZEOF },
    { "static", STATIC },
    { "static_cast", STATIC_CAST },
    { "struct", STRUCT },
    { "switch", SWITCH },
    { "template", TEMPLATE },
    { "this", THIS },
    { "__thread", THREAD },
    { "throw", THROW },
    { "true", TRUE },
    { "try", TRY },
    { "typedef", TYPEDEF },
    { "typeof", TYPEOF },				// gcc specific
    { "__typeof", TYPEOF },				// gcc specific
    { "__typeof__", TYPEOF },				// gcc specific
    { "typeid", TYPEID },
    { "typename", TYPENAME },
    { "union", UNION },
    { "unsigned", UNSIGNED },
    { "using", USING },
    { "virtual", VIRTUAL },
    { "void", VOID },
    { "volatile", VOLATILE },
    { "__volatile", VOLATILE },				// gcc specific
    { "__volatile__", VOLATILE },			// gcc specific
    { "wchar_t", WCHAR_T },
    { "__wchar_t", WCHAR_T },				// gcc specific
    { "while", WHILE },

    { "and", AND_AND },					// alternate operator names
    { "and_eq", AND_ASSIGN },
    { "bitand", '&' },
    { "bitor", '|' },
    { "compl", '~' },
    { "not", '!' },
    { "not_eq", NE },
    { "or", OR_OR },
    { "or_eq", OR_ASSIGN },
    { "xor", '^' },
    { "xor_eq", XOR_ASSIGN },

// uC++ specific

    { "_Accept", ACCEPT },
    { "_AcceptReturn", ACCEPTRETURN },
    { "_AcceptWait", ACCEPTWAIT },
    { "_At", AT },
    { "_CatchResume", CATCHRESUME },
    { "_Coroutine", COROUTINE },
    { "_Disable", DISABLE },
    { "_Else", UELSE },
    { "_Enable", ENABLE },
    { "_Event", EVENT },
    { "_Mutex", MUTEX },
    { "_Nomutex", NOMUTEX },
    { "_PeriodicTask", PTASK },
    { "_RealTimeTask", RTASK },
    { "_Resume", RESUME },
    { "_Select", SELECT },
    { "_SporadicTask", STASK },
    { "_Task", TASK },
    { "_Throw", UTHROW },
    { "_Timeout", TIMEOUT },
    { "_When", WHEN },
    { "_With", WITH },
    { NULL, 0 }
};

// Local Variables: //
// compile-command: "make install" //
// End: //
