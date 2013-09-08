//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// key.h --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:39:05 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Feb 20 14:10:19 2013
// Update Count     : 70
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


#ifndef __KEY_H__
#define __KEY_H__

struct keyword_t {
    const char *text;
    int value;
};

extern keyword_t key[];

enum key_value_t {
    ASM = 512,
    ATOMIC,
    ATTRIBUTE,						// gcc specific
    AUTO,
    BOOL,
    BREAK,
    CASE,
    CATCH,
    CHAR,
    CLASS,
    COMPLEX,						// gcc specific
    CONST,
    CONST_CAST,
    CONTINUE,
    DEFAULT,
    DELETE,
    DO,
    DOUBLE,
    DYNAMIC_CAST,
    ELSE,
    ENUM,
    EXPLICIT,
    EXPORT,
    EXTENSION,						// gcc specific
    EXTERN,
    FALSE,
    FLOAT,
    FOR,
    FRIEND,
    GOTO,
    IF,
    INLINE,
    INT,
    LONG,
    MUTABLE,
    NAMESPACE,
    NEW,
    OPERATOR,
    PRIVATE,
    PROTECTED,
    PUBLIC,
    REGISTER,
    REINTERPRET_CAST,
    RESTRICT,
    RETURN,
    SHORT,
    SIGNED,
    SIZEOF,
    STATIC,
    STATIC_CAST,
    STRUCT,
    SWITCH,
    TEMPLATE,
    THIS,
    THREAD,
    THROW,
    TRUE,
    TRY,
    TYPEDEF,
    TYPEOF,						// gcc specific
    TYPEID,
    TYPENAME,
    UNION,
    UNSIGNED,
    USING,
    VIRTUAL,
    VOID,
    VOLATILE,
    WCHAR_T,
    WHILE,

    ACCEPT,
    ACCEPTRETURN,
    ACCEPTWAIT,
    AT,
    CATCHRESUME,
    COROUTINE,
    DISABLE,
    UELSE,
    ENABLE,
    EVENT,
    MUTEX,
    NOMUTEX,
    PTASK,
    RESUME,
    RTASK,
    SELECT,
    STASK,
    TASK,
    UTHROW,
    TIMEOUT,
    WITH,
    WHEN,

    CONN_OR,						// pseudo values (i.e., no associated keywords) denotes kinds of definitions
    CONN_AND,
    SELECT_LP,
    SELECT_RP,
    MEMBER,
    ROUTINE,
};

#endif // __KEY_H__

// Local Variables: //
// compile-command: "make install" //
// End: //
