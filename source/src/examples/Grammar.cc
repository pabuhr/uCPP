//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// Grammar.cc -- Extended finite-state machine to match language
// 
// Author           : Peter A. Buhr
// Created On       : Wed Mar 20 14:11:27 1991
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 18 11:05:51 2010
// Update Count     : 96
// 

#include <fstream>
using std::ifstream;
using std::ofstream;
#include <iostream>
using std::istream;
using std::ostream;
using std::cout;
using std::cerr;
using std::cin;
using std::endl;

//  This grammar parses the language ab+c*d, where the number of b's must be exactly one greater than the number of
//  c's. The coroutine has 3 states:
//  
//  CONT  => continue parsing
//  ERROR => error in string
//  MATCH => string occurs in language

#define CONT	0
#define ERROR	1
#define MATCH	2

_Coroutine grammar {
    char ch;
    int ok;

    void main() {
	int bcnt, ccnt;

	ok = CONT;
	if ( ch == 'a' ) {
	    bcnt = 0;
	    for ( ;; ) {
		suspend();
	      if ( ch != 'b' ) break;
		bcnt += 1;
	    } // for
	    ccnt = 0;
	    for ( ;; ) {
	      if ( ch != 'c' ) break;
		ccnt += 1;
	      if ( bcnt < ccnt + 1 ) break;
		suspend();
	    } // for
	    ok = ( ch == 'd' && bcnt == ccnt + 1 ) ? MATCH : ERROR;
	} else {
	    ok = ERROR;
	} // if
    }; // grammar::main
  public:
    int check( char ch ) {
	grammar::ch = ch;
	resume();
	return ok;
    }; // grammer::check
}; // grammar

void uMain::main() {
    istream *input;
    grammar gram;
    int status, i, state;
    char string[256];
    
    switch ( argc ) {
      case 1:
	input = &cin;
	break;
      case 2:
	input = new ifstream( argv[1] );
	break;
      default:
	cerr << "Usage:" << argv[0] << " input-file" << endl;
	exit( EXIT_FAILURE );
    } // switch
    
    for ( ;; ) {
	*input >> string;				// input a string delimited by whitespace
      if ( input->eof() ) break;
	cout << "string:\"";
	{
	    grammar gram;

	    for ( i = 0; i < strlen(string) + 1; i += 1 ) { // pass the string termination character, if necessary
		state = gram.check( string[i] );
		if ( string[i] != '\0' ) cout << string[i]; // don't print the terminator character
	      if ( state == ERROR ) {
		    cout << "\" is NOT in language" << endl;
		    break;
		} // exit
	      if ( state == MATCH ) {
		    cout << "\" is in language" << endl;
		    break;
		} // exit
	    } // for
	}
    } // for

    if ( input != &cin ) delete input;			// close file
} // uMain::main

// Local Variables: //
// compile-command: "u++ Grammar.cc" //
// End: //
