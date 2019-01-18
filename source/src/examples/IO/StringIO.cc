//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2003
// 
// StringIO.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Wed Jan 22 12:32:28 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jan 13 09:04:02 2019
// Update Count     : 11
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

#include <uFile.h>
#include <fstream>
#include <iostream>
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;
#include <iomanip>
using std::skipws;
#include <string>
using std::string;

int main( int argc, char *argv[] ) {
    istream *infile;									// pointer to input stream
    ostream *outfile = &cout;							// pointer to output stream; default to cout
    string str;

    switch ( argc ) {
      case 3:
        try {
            outfile = new ofstream( argv[2] );			// open the outfile file
        } catch( uFile::Failure & ) {
            cerr << "Could not open output file \"" << argv[2] << "\"" << endl;
            exit( EXIT_FAILURE );						// TERMINATE!
        } // try
        // fall through to handle input file
      case 2:
        try {
            infile = new ifstream( argv[1] );			// open the first input file
        } catch( uFile::Failure & ) {
            cerr << "Could not open input file \"" << argv[1] << "\"" << endl;
            exit( EXIT_FAILURE );						// TERMINATE!
        } // try
        break;
      default:                                          // must specify an input file
        cerr << "Usage: " << argv[0] << " input-file [output-file]" << endl;
        exit( EXIT_FAILURE );							// TERMINATE!
    } // swtich

    *infile >> skipws;									// turn off white space skipping during input

    for ( ;; ) {                                        // copy input file to output file
        getline( *infile, str );                        // read a line
      if ( infile->eof() ) break;
        *outfile << str << endl;                        // write a line
    } // for
    delete infile;
    if ( outfile != &cout ) delete outfile;				// do not delete cout!
}

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ StringIO.cc" //
// End: //
