//                              -*- Mode: C++ -*-
//
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2003
//
// cc1plus.cc --
//
// Author           : Peter A Buhr
// Created On       : Tue Feb 25 09:04:44 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Sep  3 14:49:48 2018
// Update Count     : 222
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


#include <iostream>
using std::cerr;
using std::endl;
#include <string>
using std::string;
#include <cstdio>					// stderr, stdout, perror, fprintf
#include <cstdlib>					// getenv, exit, mkstemp
#include <unistd.h>					// execvp, fork, unlink
#include <sys/wait.h>					// wait
#if defined( __solaris__ )
#include <signal.h>					// signal
#endif // __solaris__

//#define __U_DEBUG_H__
#include "debug.h"

//#define CLANG
string compiler_name( CCAPP );				// path/name of C compiler

string D__U_GCC_BPREFIX__( "-D__U_GCC_BPREFIX__=" );

char tmpname[] = P_tmpdir "/uC++XXXXXX";
int tmpfilefd = -1;


bool prefix( string arg, string pre ) {
    return arg.substr( 0, pre.size() ) == pre;
} // prefix


void checkEnv( const char *args[], int &nargs ) {
    char *value;

    value = getenv( "__U_COMPILER__" );
    if ( value != nullptr ) {
	compiler_name = value;
	uDEBUGPRT( cerr << "env arg:\"" << compiler_name << "\"" << endl; )
    } // if

    value = getenv( "__U_GCC_MACHINE__" );
    if ( value != nullptr ) {
	args[nargs] = ( *new string( value ) ).c_str(); // pass argument along
	uDEBUGPRT( cerr << "env arg:\"" << args[nargs] << "\"" << endl; )
	nargs += 1;
    } // if

    value = getenv( "__U_GCC_VERSION__" );
    if ( value != nullptr ) {
	args[nargs] = ( *new string( value ) ).c_str(); // pass argument along
	uDEBUGPRT( cerr << "env arg:\"" << args[nargs] << "\"" << endl; )
	nargs += 1;
    } // if
} // checkEnv


void rmtmpfile() {
    if ( unlink( tmpname ) == -1 ) {			// remove tmpname
	perror ( "uC++ Translator error: cpp failed" );
	exit( EXIT_FAILURE );
    } // if
    tmpfilefd = -1;					// mark closed
} // rmtmpfile


void sigTermHandler( int ) {
    if ( tmpfilefd != -1 ) {				// RACE, file created ?
	rmtmpfile();					// remove
	exit( EXIT_FAILURE );				// terminate 
    } // if
} // sigTermHandler


void Stage1( const int argc, const char * const argv[] ) {
    int code;

    string arg;
    string bprefix;

    const char *cpp_in = nullptr;
    const char *cpp_out = nullptr;

    bool upp_flag = false;
    bool cpp_flag = false;
    const char *o_name = nullptr;

    const char *args[argc + 100];			// leave space for 100 additional cpp command line values
    int nargs = 1;					// number of arguments in args list; 0 => command name
    const char *uargs[20];				// leave space for 20 additional u++-cpp command line values
    int nuargs = 1;					// 0 => command name

    signal( SIGINT,  sigTermHandler );
    signal( SIGTERM, sigTermHandler );

    uDEBUGPRT( cerr << "Stage1" << endl; )
    checkEnv( args, nargs );				// arguments passed via environment variables
    uDEBUGPRT(
	for ( int i = 1; i < argc; i += 1 ) {
	    cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl;
	} // for
    )

    // process command-line arguments

    for ( int i = 1; i < argc; i += 1 ) {
	arg = argv[i];
	if ( prefix( arg, "-" ) ) {
	    // strip g++ flags that are inappropriate or cause duplicates in subsequent passes

	    if ( arg == "-quiet" ) {
	    } else if ( arg == "-imultilib" || arg == "-imultiarch" ) {
		i += 1;					// and the argument
	    } else if ( prefix( arg, "-A" ) ) {
	    } else if ( prefix( arg, "-D__GNU" ) ) {
	    //********
	    // GCC 5.6.0 SEPARATED THE -D FROM THE ARGUMENT!
	    //********
	    } else if ( arg == "-D" && prefix( argv[i + 1], "__GNU" ) ) {
		i += 1;					// and the argument

	    // strip flags controlling cpp step

	    } else if ( arg == "-D__U_CPP__" ) {
		cpp_flag = true;
	    } else if ( arg == "-D" && string( argv[i + 1] ) == "__U_CPP__" ) {
		i += 1;					// and the argument
		cpp_flag = true;
	    } else if ( arg == "-D__U_UPP__" ) {
		upp_flag = true;
	    } else if ( arg == "-D" && string( argv[i + 1] ) == "__U_UPP__" ) {
		i += 1;					// and the argument
		upp_flag = true;
	    } else if ( prefix( arg, D__U_GCC_BPREFIX__ ) ) {
		bprefix = arg.substr( D__U_GCC_BPREFIX__.size() );
	    } else if ( arg == "-D" && prefix( argv[i + 1], D__U_GCC_BPREFIX__.substr(2) ) ) {
		bprefix = string( argv[i + 1] ).substr( D__U_GCC_BPREFIX__.size() - 2 );
		i += 1;					// and the argument
		uDEBUGPRT( cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl; )

	    // u++ flags controlling the u++-cpp step

	    } else if ( arg == "-D__U_YIELD__" || arg == "-D__U_VERIFY__" || arg == "-D__U_PROFILE__" || arg == "-D__U_STD_CPP11__" ) {
		args[nargs] = argv[i];			// pass the flag along to cpp
		nargs += 1;
		uargs[nuargs] = argv[i];		// pass the flag along to upp
		nuargs += 1;
	    } else if ( arg == "-D" && ( string( argv[i + 1] ) == "__U_YIELD__" || string( argv[i + 1] ) == "__U_VERIFY__" || string( argv[i + 1] ) == "__U_PROFILE__" || string( argv[i + 1] ) == "__U_STD_CPP11__" ) ) {
		args[nargs] = argv[i];			// pass the flag along to cpp
		nargs += 1;
		args[nargs] = argv[i + 1];		// pass argument along to cpp
		nargs += 1;
		uargs[nuargs] = argv[i];		// pass the flag along to upp
		nuargs += 1;
		uargs[nuargs] = argv[i + 1];		// pass argument along to upp
		nuargs += 1;
		i += 1;					// and the argument

	    // all other flags

#ifdef CLANG
	    } else if ( arg == "-fworking-directory" ) {
#endif // CLANG
	    } else if ( arg == "-o" ) {
		i += 1;
		o_name = argv[i];
	    } else {
		args[nargs] = argv[i];			// pass the flag along
		nargs += 1;
		// CPP flags with an argument
		if ( arg == "-D" || arg == "-U" || arg == "-I" || arg == "-MF" || arg == "-MT" || arg == "-MQ" ||
		     arg == "-include" || arg == "-imacros" || arg == "-idirafter" || arg == "-iprefix" ||
		     arg == "-iwithprefix" || arg == "-iwithprefixbefore" || arg == "-isystem" || arg == "-isysroot" ) {
		    i += 1;
		    args[nargs] = argv[i];		// pass argument along
		    nargs += 1;
		    uDEBUGPRT( cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl; )
		} else if ( arg == "-MD" || arg == "-MMD" ) {
		    args[nargs] = "-MF";		// insert before file
		    nargs += 1;
		    i += 1;
		    args[nargs] = argv[i];		// pass argument along
		    nargs += 1;
		    uDEBUGPRT( cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl; )
		} // if
	    } // if
	} else {					// obtain input and possibly output files
	    if ( cpp_in == nullptr ) {
		cpp_in = argv[i];
		uDEBUGPRT( cerr << "cpp_in:\"" << cpp_in << "\"" << endl; )
	    } else if ( cpp_out == nullptr ) {
		cpp_out = argv[i];
		uDEBUGPRT( cerr << "cpp_out:\"" << cpp_out << "\""<< endl; )
	    } else {
		cerr << "Usage: " << argv[0] << " input-file [output-file] [options]" << endl;
		exit( EXIT_FAILURE );
	    } // if
	} // if
    } // for

    uDEBUGPRT(
	cerr << "args:";
	for ( int i = 1; i < nargs; i += 1 ) {
	    cerr << " " << args[i];
	} // for
	if ( cpp_in != nullptr ) cerr << " " << cpp_in;
	if ( cpp_out != nullptr ) cerr << " " << cpp_out;
	cerr << endl;
    )

    if ( cpp_in == nullptr ) {
	cerr << "Usage: " << argv[0] << " input-file [output-file] [options]" << endl;
	exit( EXIT_FAILURE );
    } // if

    if ( cpp_flag ) {
	// The -E flag is specified on the u++ command so only run the preprocessor and output is written to standard
	// output or -o. The call to u++ has a -E so it does not have to be added to the argument list.

	args[0] = compiler_name.c_str();
	args[nargs] = cpp_in;
	nargs += 1;
	if ( o_name != nullptr ) {			// location for output
	    args[nargs] = "-o";
	    nargs += 1;
	    args[nargs] = o_name;
	    nargs += 1;
	} // if
	args[nargs] = nullptr;				// terminate argument list

	uDEBUGPRT(
	    cerr << "nargs: " << nargs << endl;
	    for ( int i = 0; args[i] != nullptr; i += 1 ) {
		cerr << args[i] << " ";
	    } // for
	    cerr << endl;
	)

	execvp( args[0], (char *const *)args );		// should not return
	perror( "uC++ Translator error: cpp level, execvp" );
	exit( EXIT_FAILURE );
    } // if

    // Create a temporary file to store output of the C preprocessor.

    tmpfilefd = mkstemp( tmpname );
    if ( tmpfilefd == -1 ) {
	perror( "uC++ Translator error: cpp level, mkstemp" );
	exit( EXIT_FAILURE );
    } // if

    uDEBUGPRT( cerr << "tmpname:" << tmpname << " tmpfilefd:" << tmpfilefd << endl; )

    // Run the C preprocessor and save the output in tmpfile.

    if ( fork() == 0 ) {				// child process ?
	// -o xxx.ii cannot be used to write the output file from cpp because no output file is created if cpp detects
	// an error (e.g., cannot find include file). Whereas, output is always generated, even when there is an error,
	// when cpp writes to stdout. Hence, stdout is redirected into the temporary file.
	if ( freopen( tmpname, "w", stdout ) == nullptr ) { // redirect stdout to tmpname
	    perror( "uC++ Translator error: cpp level, freopen" );
	    exit( EXIT_FAILURE );
	} // if

#ifdef CLANG	
	args[0] = "clang-5.0";
#else	
	args[0] = compiler_name.c_str();
#endif // CLANG
	args[nargs] = cpp_in;				// input to cpp
	nargs += 1;
	args[nargs] = nullptr;				// terminate argument list

	uDEBUGPRT(
	    cerr << "cpp nargs: " << nargs << endl;
	    for ( int i = 0; args[i] != nullptr; i += 1 ) {
		cerr << args[i] << " ";
	    } // for
	    cerr << endl;
	)

	execvp( args[0], (char *const *)args );		// should not return
	perror( "uC++ Translator error: cpp level, execvp" );
	exit( EXIT_FAILURE );
    } // if

    wait( &code );					// wait for child to finish

    uDEBUGPRT( cerr << "return code from cpp:" << WEXITSTATUS(code) << endl; )

    if ( WIFSIGNALED(code) != 0 ) {			// child failed ?
	rmtmpfile();					// remove tmpname
	cerr << "uC++ Translator error: cpp failed with signal " << WTERMSIG(code) << endl;
	exit( EXIT_FAILURE );
    } // if

    if ( WEXITSTATUS(code) != 0 ) {			// child error ?
	rmtmpfile();					// remove tmpname
	exit( WEXITSTATUS( code ) );			// do not continue
    } // if

    // If -U++ flag specified, run the u++-cpp preprocessor on the temporary file, and output is written to standard
    // output.  Otherwise, run the u++-cpp preprocessor on the temporary file and save the result into the output file.

    if ( fork() == 0 ) {				// child runs CFA
	uargs[0] = ( *new string( bprefix + "/u++-cpp" ) ).c_str();

	uargs[nuargs] = tmpname;
	nuargs += 1;
	if ( o_name != nullptr ) {
	    uargs[nuargs] = o_name;
	    nuargs += 1;
	} else if ( ! upp_flag ) {			// run u++-cpp ?
	    uargs[nuargs] = cpp_out;
	    nuargs += 1;
	} // if
	uargs[nuargs] = nullptr;			// terminate argument list

	uDEBUGPRT(
	    cerr << "u++-cpp nuargs: " << o_name << " " << upp_flag << " " << nuargs << endl;
	    for ( int i = 0; uargs[i] != nullptr; i += 1 ) {
		cerr << uargs[i] << " ";
	    } // for
	    cerr << endl;
	)

	execvp( uargs[0], (char * const *)uargs );	// should not return
	perror( "uC++ Translator error: cpp level, execvp" );
	exit( EXIT_FAILURE );
    } // if

    wait( &code );					// wait for child to finish

    uDEBUGPRT( cerr << "return code from u++-cpp:" << WEXITSTATUS(code) << endl; )

    // Must unlink here because file must exist across execvp.
    rmtmpfile();					// remove tmpname

    if ( WIFSIGNALED(code) ) {				// child failed ?
	cerr << "uC++ Translator error: u++-cpp failed with signal " << WTERMSIG(code) << endl;
	exit( EXIT_FAILURE );
    } // if

    exit( WEXITSTATUS(code) );
} // Stage1


void Stage2( const int argc, const char * const * argv ) {
    string arg;

    const char *cpp_in = nullptr;

    const char *args[argc + 100];			// leave space for 100 additional u++ command line values
    int nargs = 1;					// number of arguments in args list; 0 => command name

    uDEBUGPRT( cerr << "Stage2" << endl; )
    checkEnv( args, nargs );				// arguments passed via environment variables
    uDEBUGPRT(
	for ( int i = 1; i < argc; i += 1 ) {
	    cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl;
	} // for
    )

    // process all the arguments

    for ( int i = 1; i < argc; i += 1 ) {
	uDEBUGPRT( cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl; )
	arg = argv[i];
	uDEBUGPRT( cerr << "arg:\"" << arg << "\"" << endl; )
	if ( prefix( arg, "-" ) ) {
	    // strip inappropriate flags

	    if ( arg == "-quiet" || arg == "-version" || arg == "-fpreprocessed" ||
		 // Currently uC++ does not suppose precompiled .h files.
		 prefix( arg, "--output-pch" ) ) {

	    // strip inappropriate flags with an argument

	    } else if ( arg == "-auxbase" || arg == "-auxbase-strip" || arg == "-dumpbase" ) {
		i += 1;
		uDEBUGPRT( cerr << "arg:\"" << argv[i] << "\"" << endl; )

	    // all other flags

	    } else {
		args[nargs] = argv[i];			// pass the flag along
		nargs += 1;
		if ( arg == "-o" ) {
		    i += 1;
		    args[nargs] = argv[i];		// pass argument along
		    nargs += 1;
		    uDEBUGPRT( cerr << "arg:\"" << argv[i] << "\"" << endl; )
		} // if
	    } // if
	} else {					// obtain input and possibly output files
	    if ( cpp_in == nullptr ) {
		cpp_in = argv[i];
		uDEBUGPRT( cerr << "cpp_in:\"" << cpp_in << "\"" << endl; )
	    } else {
		cerr << "Usage: " << argv[0] << " input-file [output-file] [options]" << endl;
		exit( EXIT_FAILURE );
	    } // if
	} // if
    } // for

    uDEBUGPRT(
	cerr << "args:";
	for ( int i = 1; i < nargs; i += 1 ) {
	    cerr << " " << args[i];
	} // for
	cerr << endl;
	if ( cpp_in != nullptr ) cerr << " " << cpp_in;
    )

#ifdef CLANG
    args[0] = "clang-5.0";
    args[nargs] = "-fno-integrated-as";
    nargs += 1;
#else
    args[0] = compiler_name.c_str();
#endif // CLANG
    args[nargs] = "-S";					// only compile and put assembler output in specified file
    nargs += 1;
    args[nargs] = cpp_in;
    nargs += 1;
    args[nargs] = nullptr;				// terminate argument list

    uDEBUGPRT(
	cerr << "stage2 nargs: " << nargs << endl;
	for ( int i = 0; args[i] != nullptr; i += 1 ) {
	    cerr << args[i] << " ";
	} // for
	cerr << endl;
    )

    execvp( args[0], (char * const *)args );		// should not return
    perror( "uC++ Translator error: cpp level, execvp" );
    exit( EXIT_FAILURE );				// tell gcc not to go any further
} // Stage2


int main( const int argc, const char * const argv[] ) {
    string arg = argv[1];

    // Currently, stage 1 starts with flag -E and stage 2 with flag -fpreprocessed.

    if ( arg == "-E" ) {
	Stage1( argc, argv );
    } else if ( arg == "-fpreprocessed" ) {
	Stage2( argc, argv );
    } else {
	cerr << "Usage: " << argv[0] << " input-file [output-file] [options]" << endl;
	exit( EXIT_FAILURE );
    } // if
} // main


// Local Variables: //
// compile-command: "make install" //
// End: //
