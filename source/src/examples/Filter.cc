//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// Filter.cc -- 
// 
// Author           : Richard A. Stroobosscher
// Created On       : Mon Jun  3 10:17:16 1991
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 31 20:38:24 2005
// Update Count     : 48
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
#include <iomanip>
using std::skipws;

static const int blank = ' ';
static const char EOD = '\377';

_Coroutine filter {					// abstract class for all filters
  protected:
    char ch;
  public:
    void put( char c ) {
	ch = c;
	resume();
    } // filter::put
}; // filter

_Coroutine reader : public filter {
  private:
    istream *in;
    filter *next;
  private:
    void main();
  public:
    reader( istream *i, filter *n ) {
	in = i;
	next = n;
	resume();
    } // reader::reader
}; // reader

void reader::main() {
    for ( ;; ) {
	*in >> ch;
	if ( in->eof() ) ch = EOD;
	next->put( ch );
      if ( ch == EOD ) break;
    } // for
} // reader::main

_Coroutine writer : public filter {
  private:
    ostream *out;
  private:
    void main();
  public:
    writer( ostream *o ) {
	out = o;
    } // writer::writer
}; // writer

void writer::main() {
    for ( ;; ) {
      if ( ch == EOD ) break;
	*out << ch;
	suspend();
    } // for
} // writer::main

_Coroutine eliminate_white_space : public filter {
  private:
    filter *next;
  private:
    void main();
  public:
    eliminate_white_space( filter *n ) {
	next = n;
    } // eliminate_white_space::eliminate_white_space
}; // eliminate_white_space

void eliminate_white_space::main() {
    for ( ;; ) {
	
	// skip the leading blanks
	
	while ( ch == blank ) {
	    suspend();
	} // for

	// pass along all non-blank chararacters

	for ( ;; ) {
	    next->put( ch );
	    suspend();
	  if ( ch == blank ) break;
	  if ( ch == EOD ) break;
	} // for

      if ( ch == EOD ) break;
	
	// if we encounter a blank, keep eating them until we hit a non-blank character.
	// if we hit a new line, swallow the blank.

	while ( ch == blank ) {
	    suspend();
	} // while

	// process the next non-blank character
	
      if ( ch == EOD ) {
	    next->put( ch );
	    break;
	} // exit

	if ( ch == '\n' ) {
	    next->put( ch );
	} else {
	    next->put( blank );
	    next->put( ch );
	} // if

	suspend();
	
    } // for
} // eliminate_white_space::main

_Coroutine expand_tab : public filter {
  private:
    int space;
    filter *next;
  private:
    void main();
  public:
    expand_tab( int s, filter *n ) {
	space = s;
	next = n;
    } // expand_tab::expand_tab
}; // expand_tab

void expand_tab::main() {
    int column = 0;
    
    for ( ;; ) {
      if ( ch == EOD ) {
	    next->put( ch );
	    break;
	} // exit

	if ( ch == '\n' ) {
	    column = 0;
	    next->put( ch );
	    suspend();
	} else if ( ch == '\t' ) {
	    if ( space != 0 ) {
		ch = blank;
		
		// if you are in the right column for a tab, just pass
		// the character to the next routine, otherwise pass a
		// blank to the next routine until you reach a tab column.

		if ( column % space == 0 ) {
		    next->put( ch );
		    column += 1;
		} // if
		
		for ( ; column % space != 0; column += 1 ) {
		    next->put( ch );
		} // for
	    } // if
	    suspend();
	} else {
	    next->put( ch );
	    column += 1;
	    suspend();
	} // if
    } // for
} // expand_tab::main

_Coroutine convert_to_hex : public filter {
  private:
    filter *next;
    int many;
  private:
    void main();
  public:
    convert_to_hex( filter *n, int m ) {
	next = n;
	many = m;
    } // convert_to_hex::convert_to_hex
}; // convert_to_hex

void convert_to_hex::main() {
    const char *s = "0123456789abcdef";
    
    for ( ;; ) {
	for ( int i = 0; i < many; i += 1 ) {
	  if ( ch == EOD ) {
		next->put( '\n' );
		next->put( ch );
		break;
	    } // exit
	    next->put( s[(ch & 0xf0) >> 4] );
	    next->put( s[(ch & 0x0f)] );
	    next->put( blank );
	    suspend();
	} // for
      if ( ch == EOD ) break;
	next->put( '\n' );
    } // for
} // convert_to_hex::main

_Coroutine scramble : public filter {
  private:
    filter *next;
    char *key;
  private:
    void main();
  public:
    scramble( filter *n, char *k ) {
	next = n;
	key = k;
    } // scramble::scramble
}; // scramble
    
void scramble::main() {
    for ( ;; ) {
	for ( char *p = key; *p != 0; p += 1 ) {
	  if ( ch == EOD ) {
		next->put( ch );
		break;
	    } // if
	    next->put( ch ^ *p );
	    suspend();
	} // for
      if ( ch == EOD ) break;
    } // for
} // scramble::main

_Coroutine reverse : public filter {
  private:
    filter *next;
  private:
    void main();
  public:
    reverse( filter *n ) {
	next = n;
    } // reverse::reverse
}; // reverse

void reverse::main() {
    for ( ;; ) {
      if ( ch == EOD ) break;
	if ( islower( ch ) ) {
	    next->put( toupper( ch ) );
	} else if ( isupper( ch ) ) {
	    next->put( tolower( ch ) );
	} else {
	    next->put( ch );
	} // if
	suspend();
    } // for
} // reverse::main

void uMain::main() {
    filter *fa[argc];

    cin >> skipws;					// turn off white space skipping
    
    writer w( &cout );

    filter *next = &w;

    for ( int i = argc - 1; i != 0; i -= 1 ) {
	fa[i] = NULL;
	if ( argv[i][0] == '-' ) {
	    switch ( argv[i][1] ) {
	      case 'w':
		fa[i] = next = new eliminate_white_space( next );
		break;
	    case 't':
		fa[i] = next = new expand_tab( atoi( argv[i+1] ), next );
		break;
	    case 'h':
		fa[i] = next = new convert_to_hex( next, atoi( argv[i+1] ) );
		break;
	    case 'e':
		fa[i] = next = new scramble( next, argv[i+1] );
		break;
	    case 'r':
		fa[i] = next = new reverse( next );
		break;
	      default:
		cerr << argv[0] << ": unrecognized option: " << argv[i] << endl;
		break;
	    } // switch
	} // if
    } // for

    reader r( &cin, next );
    
    for ( int i = argc - 1; i != 0; i -= 1 ) {
	if ( fa[i] != NULL ) delete fa[i];
    } // for
} // uMain::main

// Local Variables: //
// compile-command: "u++ Filter.cc" //
// End: //
