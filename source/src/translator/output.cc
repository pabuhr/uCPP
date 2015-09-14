//                              -*- Mode: C++ -*-
//
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr and Richard A. Stroobosscher 1994
//
// output.c --
//
// Author           : Richard A. Stroobosscher
// Created On       : Tue Apr 28 15:09:30 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Apr 29 20:45:09 2015
// Update Count     : 200
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

#include <cstdio>					// EOF

#include "uassert.h"
#include "main.h"
#include "key.h"
#include "hash.h"
#include "token.h"
#include "output.h"

#include <cstring>					// strcpy, strlen

char *file = NULL;
token_t *file_token = NULL;
unsigned int line = 1;

void context() {
    token_t *p;
    int i;

    cerr << "=====>\"";
    // backup up to 10 tokens
    for ( i = 0, p = ahead; i < 10 && p->aft != NULL; i += 1, p = p->aft );
    // print up to 10 tokens before problem area
    for ( ; p != ahead; p = p->fore ) {
      if ( p->hash == NULL ) continue;
	cerr << p->hash->text << " ";
    } // for
    cerr << " @ " << ahead->hash->text << " @ ";
    // print up to 10 tokens after problem area
    for ( i = 0, p = ahead->fore; i < 10 && p != NULL; i += 1, p = p->fore ) {
      if ( p->hash == NULL ) continue;
	cerr << p->hash->text << " ";
    } // for
    cerr << "\"" << endl;
} // context

void sigSegvBusHandler( int sig ) {
    cerr << "uC++ Translator error: fatal problem during parsing." << endl <<
	"Probable cause is mismatched braces, missing terminating quote, or use of an undeclared type name." << endl <<
	"Possible area where problem occurred:" << endl;
    context();
    exit( EXIT_SUCCESS );
} // sigSegvBusHandler

void parse_directive( char *text, char *&file, unsigned int &line ) {
    char *c = text + 1;					// get past '#'
    while ( *c == ' ' || *c == '\t' ) c += 1;		// skip whitespace
    if ( isdigit( *c ) ) {				// must be a line directive
	line = 0;
	while ( isdigit( *c ) ) {
	    line = line * 10 + ( *c - '0' );
	    c += 1;
	} // while
	while ( *c == ' ' || *c == '\t' ) c += 1;	// skip whitespace
	if ( *c == '\"' ) {				// must be a file directive
	    char *s = c + 1;				// remember where the string begins
	    c = s;
	    while ( *c != '\"' ) c += 1;		// look for the end of the file name
	    *c = '\0';					// terminate the string containing the file name
	    if ( file != NULL ) delete [] file;		// deallocate old string
	    file = new char[ strlen( s ) + 1 ];		// allocate new string
	    strcpy( file, s );				// copy the file name into this string
	    *c = '\"';					// fill in the end quote again
	} // if
    } else {
	line += 1;					// it was a normal directive, increment the line number
    } // if
} // parse_directive

// The routine 'output' converts a token value into text.  A considerable amount of effort is taken to keep track of the
// current file name and line number so that when error and warning messages appear, the exact origin of those messages
// can be displayed.

void putoutput( token_t *token ) {
    uassert( token != NULL );
    uassert( token->hash != NULL );
    uassert( token->hash->text != NULL );

    switch ( token->value ) {
      case '\n':
      case '\r':
	line += 1;
	*yyout << token->hash->text;
	break;
      case '#':
	parse_directive( token->hash->text, file, line );
	file_token = token;
	*yyout << token->hash->text;
	break;
      case ERROR:
	cerr << file << ":" << line << ": uC++ Translator error: " << token->hash->text << endl;
	error = true;
	break;
      case WARNING:
	cerr << file << ":" << line << ": uC++ Translator warning: " << token->hash->text << endl;
	break;
      case USER_LITERAL:
	*yyout << token->hash->text;			// no space
	break;
      case AT:						// do not print these keywords
      case CONN_OR:
      case CONN_AND:
      case SELECT_LP:
      case SELECT_RP:
      case MUTEX:
      case NOMUTEX:
      case WHEN:
      case CATCHRESUME:
	break;
      case EVENT:
      case COROUTINE:
      case TASK:
      case PTASK:
      case RTASK:
      case STASK:
      case DISABLE:
      case ENABLE:
      case RESUME:
      case UTHROW:
      case ACCEPT:
      case ACCEPTRETURN:
      case ACCEPTWAIT:
      case SELECT:
      case TIMEOUT:
      case WITH:
	{
	    // if no code or error was generated, print an error now
	    int value = token->next_parse_token()->value;
	    if ( value != CODE && value != ERROR ) {
		cerr << file << ":" << line << ": uC++ Translator error: parse error before " << token->hash->text << "." << endl;
		error = true;
	    } // if
	    break;
	}
      default:
	*yyout << " " << token->hash->text;
	break;
    } // switch
} // putoutput

// The routine 'write_all_output' takes the stream of tokens and calls 'putoutput'
// to convert them all to a stream of text.  It then deletes each token in the
// list.

void write_all_output() {
    for ( ;; ) {
	token_t *token = token_list->remove_from_head();
      if ( token->value == EOF ) break;
	putoutput( token );
    } // for
} // write_all_output

// Local Variables: //
// compile-command: "make install" //
// End: //
