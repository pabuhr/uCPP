//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// key.h --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:39:05 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Jul 24 21:34:51 2014
// Update Count     : 90
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
    // Operators

    EQ = 256,						// ==
    NE,							// !=
    LE,							// <=
    GE,							// >=

    PLUS_ASSIGN,					// +=
    MINUS_ASSIGN,					// -=
    LSH_ASSIGN,						// <<=
    RSH_ASSIGN,						// >>=
    AND_ASSIGN,						// &=
    XOR_ASSIGN,						// ^=
    OR_ASSIGN,						// |=
    MULTIPLY_ASSIGN,					// *=
    DIVIDE_ASSIGN,					// /=
    MODULUS_ASSIGN,					// %=

    AND_AND,						// &&
    OR_OR,						// ||
    PLUS_PLUS,						// ++
    MINUS_MINUS,					// --
    RSH,						// >>
    LSH,						// <<
    GMIN,						// <? (min) gcc specific, deprecated
    GMAX,						// >? (max) gcc specific, deprecated

    ARROW,						// ->
    ARROW_STAR,						// ->*
    DOT_STAR,						// ->.

    CHARACTER,						// 'a'
    STRING,						// "abc"
    NUMBER,						// integer (oct,dec,hex) and floating-point constants

    IDENTIFIER,						// variable names
    LABEL,						// statement labels
    TYPE,						// builtin and user defined types

    DOT_DOT,						// meta, intermediate parsing state
    DOT_DOT_DOT,					// ...

    COLON_COLON,					// ::

    USER_LITERAL,					// meta, user literal name
    ERROR,						// meta, error mesage
    WARNING,						// meta, warning message
    CODE,						// meta, generated code

    // Keywords

    ALIGNAS = 512,					// C++11
    ALIGNOF,						// C++11
    ASM,
    ATOMIC,						// C11
    ATTRIBUTE,						// gcc specific
    AUTO,						// C++11
    BOOL,
    BREAK,
    CASE,
    CATCH,
    CHAR,
    CHAR16_t,
    CHAR32_t,
    CLASS,
    COMPLEX,						// gcc/c99 specific
    CONST,
    CONSTEXPR,						// C++11
    CONST_CAST,
    CONTINUE,
    DECLTYPE,						// C++11
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
    FINAL,						// C++11
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
    NOEXCEPT,						// C++11
    NULLPTR,						// C++11
    OPERATOR,
    OVERRIDE,
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
    STATIC_ASSERT,					// C++11
    STATIC_CAST,
    STRUCT,
    SWITCH,
    TEMPLATE,
    THIS,
    THREAD,
    THREAD_LOCAL,					// C++11
    THROW,
    TRUE,
    TRY,
    TYPEDEF,
    TYPEOF,						// gcc specific
    TYPEID,
    TYPENAME,
    UNDERLYING_TYPE,					// gcc specific
    UNION,
    UNSIGNED,
    USING,
    VIRTUAL,
    VOID,
    VOLATILE,
    WCHAR_T,
    WHILE,

    // uC++ specific

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
