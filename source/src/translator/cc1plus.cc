//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 2003
//
// cc1plus.cc --
//
// Author           : Peter A Buhr
// Created On       : Tue Feb 25 09:04:44 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Dec  9 13:04:55 2011
// Update Count     : 134
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


#include <string>
using std::string;
#include <cstdio>
#include <cstdlib>					// getenv, exit
#include <cstring>					// strcmp, strlen
#include <unistd.h>					// execvp, fork, unlink
#include <sys/wait.h>					// wait


//#define __U_DEBUG_H__

#ifdef __U_DEBUG_H__
#include <iostream>
using std::cerr;
using std::endl;
#endif


string compiler_name( CCAPP );				// path/name of C compiler

string D__U_GCC_BPREFIX__( "-D__U_GCC_BPREFIX__=" );


bool prefix( string arg, string pre ) {
    return arg.substr( 0, pre.size() ) == pre;
} // prefix


void checkEnv( const char *args[], int &nargs ) {
    char *value;

    value = getenv( "__U_COMPILER__" );
    if ( value != NULL ) {
	compiler_name = value;
#ifdef __U_DEBUG_H__
	cerr << "env arg:\"" << compiler_name << "\"" << endl;
#endif // __U_DEBUG_H__
    } // if

    value = getenv( "__U_GCC_MACHINE__" );
    if ( value != NULL ) {
	args[nargs] = ( *new string( value ) ).c_str(); // pass the argument along
#ifdef __U_DEBUG_H__
	cerr << "env arg:\"" << args[nargs] << "\"" << endl;
#endif // __U_DEBUG_H__
	nargs += 1;
    } // if

    value = getenv( "__U_GCC_VERSION__" );
    if ( value != NULL ) {
	args[nargs] = ( *new string( value ) ).c_str(); // pass the argument along
#ifdef __U_DEBUG_H__
	cerr << "env arg:\"" << args[nargs] << "\"" << endl;
#endif // __U_DEBUG_H__
	nargs += 1;
    } // if
} // checkEnv


void Stage1( const int argc, const char * const argv[] ) {
    int code;
    int i;

    string arg;
    string bprefix;

    const char *cpp_in = NULL;
    const char *cpp_out = NULL;

    bool upp_flag = false;
    bool cpp_flag = false;
    const char *o_name = NULL;

    const char *args[argc + 100];			// leave space for 100 additional cpp command line values
    int nargs = 1;					// number of arguments in args list; 0 => command name
    const char *uargs[20];				// leave space for 20 additional u++-cpp command line values
    int nuargs = 1;					// 0 => command name

    // process all the arguments

    checkEnv( args, nargs );				// arguments passed via environment variables

    for ( i = 1; i < argc; i += 1 ) {
#ifdef __U_DEBUG_H__
	cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl;
#endif // __U_DEBUG_H__
	arg = argv[i];
#ifdef __U_DEBUG_H__
	cerr << "arg:\"" << arg << "\"" << endl;
#endif // __U_DEBUG_H__
	if ( prefix( arg, "-" ) ) {
	    // strip g++ flags that are inappropriate or cause duplicates in subsequent passes

	    if ( arg == "-quiet" ) {
	    } else if ( arg == "-imultilib" || arg == "-imultiarch" ) {
		i += 1;					// and the argument
	    } else if ( prefix( arg, "-A" ) ) {
	    } else if ( prefix( arg, "-D__GNU" ) ) {
	    // gcc 5.6.0 separated the -D from the argument
	    } else if ( arg == "-D" && prefix( argv[i + 1], "__GNU" ) ) {
		i += 1;					// and the argument

	    // strip u++ flags controlling cpp step

	    } else if ( arg == "-D__U_CPP__" ) {
		cpp_flag = true;
	    } else if ( arg == "-D" && string( argv[i + 1] ) == "__U_CPP__" ) {
		i += 1;					// and the argument
		cpp_flag = true;
	    } else if ( arg == "-D__U_UPP__" ) {
		upp_flag = true;
	    } else if ( arg == "-D" && string( argv[i + 1] ) == "__U_UPP__" ) {
		i += 1;					// and the argument
	    } else if ( prefix( arg, D__U_GCC_BPREFIX__ ) ) {
		bprefix = arg.substr( D__U_GCC_BPREFIX__.size() );
	    } else if ( arg == "-D" && prefix( argv[i + 1], "__U_GCC_BPREFIX__=" ) ) {
		bprefix = string( argv[i + 1] ).substr( D__U_GCC_BPREFIX__.size() - 2 );
		i += 1;					// and the argument

	    // u++ flags controlling the u++-cpp step

	    } else if ( arg == "-D__U_YIELD__" || arg == "-D__U_VERIFY__" || arg == "-D__U_PROFILE__" ) {
		args[nargs] = argv[i];			// pass the flag along to cpp
		nargs += 1;
		uargs[nuargs] = argv[i];		// pass the flag along to upp
		nuargs += 1;
	    } else if ( arg == "-D" && ( string( argv[i + 1] ) == "__U_YIELD__" || string( argv[i + 1] ) == "__U_VERIFY__" || string( argv[i + 1] ) == "__U_PROFILE__" ) ) {
		args[nargs] = argv[i];			// pass the flag along to cpp
		nargs += 1;
		args[nargs] = argv[i + 1];		// pass the argument along to cpp
		nargs += 1;
		uargs[nuargs] = argv[i];		// pass the flag along to upp
		nuargs += 1;
		uargs[nuargs] = argv[i + 1];		// pass the argument along to upp
		nuargs += 1;
		i += 1;					// and the argument

	    // all other flags

	    } else if ( arg == "-o" ) {
	        i += 1;
	        o_name = argv[i];
	    } else {
		args[nargs] = argv[i];			// pass the flag along
		nargs += 1;
		// CPP flags with an argument
		if ( arg == "-D" || arg == "-I" || arg == "-MF" || arg == "-MT" || arg == "-MQ" ||
		     arg == "-include" || arg == "-imacros" || arg == "-idirafter" || arg == "-iprefix" ||
		     arg == "-iwithprefix" || arg == "-iwithprefixbefore" || arg == "-isystem" ) {
		    i += 1;
		    args[nargs] = argv[i];		// pass the argument along
		    nargs += 1;
#ifdef __U_DEBUG_H__
		    cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl;
#endif // __U_DEBUG_H__
		} else if ( arg == "-MD" || arg == "-MMD" ) {
		    args[nargs] = "-MF";		// insert before file
		    nargs += 1;
		    i += 1;
		    args[nargs] = argv[i];		// pass the argument along
		    nargs += 1;
#ifdef __U_DEBUG_H__
		    cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl;
#endif // __U_DEBUG_H__
		} // if
	    } // if
	} else {					// obtain input and possibly output files
	    if ( cpp_in == NULL ) {
		cpp_in = argv[i];
#ifdef __U_DEBUG_H__
		cerr << "cpp_in:\"" << cpp_in << "\"" << endl;
#endif // __U_DEBUG_H__
	    } else if ( cpp_out == NULL ) {
		cpp_out = argv[i];
#ifdef __U_DEBUG_H__
		cerr << "cpp_out:\"" << cpp_out << "\""<< endl;
#endif // __U_DEBUG_H__
	    } else {
		fprintf( stderr, "Usage: %s input-file [output-file] [options]\n", argv[0] );
		exit( EXIT_FAILURE );
	    } // if
	} // if
    } // for

#ifdef __U_DEBUG_H__
    cerr << "args:";
    for ( i = 1; i < nargs; i += 1 ) {
	cerr << " " << args[i];
    } // for
    if ( cpp_in != NULL ) cerr << " " << cpp_in;
    if ( cpp_out != NULL ) cerr << " " << cpp_out;
    cerr << endl;
#endif // __U_DEBUG_H__

    if ( cpp_in == NULL ) {
	fprintf( stderr, "Usage: %s input-file [output-file] [options]\n", argv[0] );
	exit( EXIT_FAILURE );
    } // if

    if ( cpp_flag ) {
	// The -E flag is specified on the u++ command so only run the preprocessor and output is written to standard
	// output or -o. The call to u++ has a -E so it does not have to be added to the argument list.

	args[0] = compiler_name.c_str();
	args[nargs] = cpp_in;
	nargs += 1;
	if ( o_name != NULL ) {				// location for output
	    args[nargs] = "-o";
	    nargs += 1;
	    args[nargs] = o_name;
	    nargs += 1;
	} // if
	args[nargs] = NULL;				// terminate argument list

#ifdef __U_DEBUG_H__
	cerr << "nargs: " << nargs << endl;
	for ( i = 0; args[i] != NULL; i += 1 ) {
	    cerr << args[i] << " ";
	} // for
	cerr << endl;
#endif // __U_DEBUG_H__

	execvp( args[0], (char *const *)args );		// should not return
	perror( "uC++ Translator error: cpp level, execvp" );
	exit( EXIT_FAILURE );
    } // if

    // Create a temporary file to store output of the C preprocessor.

    char tmpname[] = P_tmpdir "/uC++XXXXXX";
    int tmpfile = mkstemp( tmpname );
    if ( tmpfile == -1 ) {
	perror( "uC++ Translator error: cpp level, mkstemp" );
	exit( EXIT_FAILURE );
    } // if

#ifdef __U_DEBUG_H__
    cerr << "tmpname:" << tmpname << " tmpfile:" << tmpfile << endl;
#endif // __U_DEBUG_H__

    // Run the C preprocessor and save the output in tmpfile.

    if ( fork() == 0 ) {				// child process ?
	// -o xxx.ii cannot be used to write the output file from cpp because no output file is created if cpp detects
	// an error (e.g., cannot find include file). Whereas, output is always generated, even when there is an error,
	// when cpp writes to stdout. Hence, stdout is redirected into the temporary file.
	if ( freopen( tmpname, "w", stdout ) == NULL ) { // redirect stdout to tmpname
	    perror( "uC++ Translator error: cpp level, freopen" );
	    exit( EXIT_FAILURE );
	} // if
	args[0] = compiler_name.c_str();
	args[nargs] = cpp_in;				// input to cpp
	nargs += 1;
	args[nargs] = NULL;				// terminate argument list

#ifdef __U_DEBUG_H__
	cerr << "cpp nargs: " << nargs << endl;
	for ( i = 0; args[i] != NULL; i += 1 ) {
	    cerr << args[i] << " ";
	} // for
	cerr << endl;
#endif // __U_DEBUG_H__

	execvp( args[0], (char *const *)args );		// should not return
	perror( "uC++ Translator error: cpp level, execvp" );
	exit( EXIT_FAILURE );
    } // if

    wait( &code );					// wait for child to finish

#ifdef __U_DEBUG_H__
    cerr << "return code from cpp:" << WEXITSTATUS(code) << endl;
#endif // __U_DEBUG_H__

    if ( WIFSIGNALED(code) ) {				// child failed ?
	unlink( tmpname );				// remove tmpname
	fprintf( stderr, "uC++ Translator error: cpp failed with signal %d\n", WTERMSIG(code) );
	exit( EXIT_FAILURE );
    } // if

    if ( WEXITSTATUS(code) != 0 ) {			// child error ?
	unlink( tmpname );				// remove tmpname
	exit( WEXITSTATUS( code ) );			// do not continue
    } // if

    // If -U++ flag specified, run the u++-cpp preprocessor on the temporary file, and output is written to standard
    // output.  Otherwise, run the u++-cpp preprocessor on the temporary file and save the result into the output file.

    if ( upp_flag || fork() == 0 ) {			// conditional fork ?
	uargs[0] = ( *new string( bprefix + "/u++-cpp" ) ).c_str();

	uargs[nuargs] = tmpname;
	nuargs += 1;
	if ( o_name != NULL ) {
	    uargs[nuargs] = o_name;
	    nuargs += 1;
	} else if ( ! upp_flag ) {			// run u++-cpp ?
	    uargs[nuargs] = cpp_out;
	    nuargs += 1;
	} // if
	uargs[nuargs] = NULL;				// terminate argument list

#ifdef __U_DEBUG_H__
	cerr << "u++-cpp nuargs: " << nuargs << endl;
	for ( i = 0; uargs[i] != NULL; i += 1 ) {
	    cerr << uargs[i] << " ";
	} // for
	cerr << endl;
#endif // __U_DEBUG_H__

	execvp( uargs[0], (char * const *)uargs );	// should not return
	perror( "uC++ Translator error: cpp level, execvp" );
	exit( EXIT_FAILURE );
    } // if

    wait( &code );					// wait for child to finish

#ifdef __U_DEBUG_H__
    cerr << "return code from u++-cpp:" << WEXITSTATUS(code) << endl;
#endif // __U_DEBUG_H__

    // Must unlink here because file must exist across execvp.
    if ( unlink( tmpname ) == -1 ) {
	perror( "uC++ Translator error: cpp level, unlink" );
	exit( EXIT_FAILURE );
    } // if

    if ( WIFSIGNALED(code) ) {				// child failed ?
	fprintf( stderr, "uC++ Translator error: u++-cpp failed with signal %d\n", WTERMSIG(code) );
	exit( EXIT_FAILURE );
    } // if

    exit( WEXITSTATUS(code) );
} // Stage1


void Stage2( const int argc, const char * const * argv ) {
    int i;

    string arg;

    const char *cpp_in = NULL;

    const char *args[argc + 100];			// leave space for 100 additional u++ command line values
    int nargs = 1;					// number of arguments in args list; 0 => command name

    // process all the arguments

    checkEnv( args, nargs );				// arguments passed via environment variables

    for ( i = 1; i < argc; i += 1 ) {
#ifdef __U_DEBUG_H__
	cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl;
#endif // __U_DEBUG_H__
	arg = argv[i];
#ifdef __U_DEBUG_H__
	cerr << "arg:\"" << arg << "\"" << endl;
#endif // __U_DEBUG_H__
	if ( prefix( arg, "-" ) ) {
	    // strip inappropriate flags

	    if ( arg == "-quiet" || arg == "-version" || arg == "-fpreprocessed" ||
		 // Currently uC++ does not suppose precompiled .h files.
		 prefix( arg, "--output-pch" ) ) {

	    // strip inappropriate flags with an argument

	    } else if ( arg == "-auxbase" || arg == "-auxbase-strip" || arg == "-dumpbase" ) {
		i += 1;
#ifdef __U_DEBUG_H__
		cerr << "arg:\"" << argv[i] << "\"" << endl;
#endif // __U_DEBUG_H__

	    // all other flags

	    } else {
		args[nargs] = argv[i];			// pass the flag along
		nargs += 1;
		if ( arg == "-o" ) {
		    i += 1;
		    args[nargs] = argv[i];		// pass the argument along
		    nargs += 1;
#ifdef __U_DEBUG_H__
		    cerr << "arg:\"" << argv[i] << "\"" << endl;
#endif // __U_DEBUG_H__
		} // if
	    } // if
	} else {					// obtain input and possibly output files
	    if ( cpp_in == NULL ) {
		cpp_in = argv[i];
#ifdef __U_DEBUG_H__
		cerr << "cpp_in:\"" << cpp_in << "\"" << endl;
#endif // __U_DEBUG_H__
	    } else {
		fprintf( stderr, "Usage: %s input-file [output-file] [options]\n", argv[0] );
		exit( EXIT_FAILURE );
	    } // if
	} // if
    } // for

#ifdef __U_DEBUG_H__
    cerr << "args:";
    for ( i = 1; i < nargs; i += 1 ) {
	cerr << " " << args[i];
    } // for
    cerr << endl;
    if ( cpp_in != NULL ) cerr << " " << cpp_in;
#endif // __U_DEBUG_H__

    args[0] = compiler_name.c_str();
    args[nargs] = "-S";					// only compile and put assembler output in specified file
    nargs += 1;
    args[nargs] = cpp_in;
    nargs += 1;
    args[nargs] = NULL;					// terminate argument list

#ifdef __U_DEBUG_H__
    cerr << "stage2 nargs: " << nargs << endl;
    for ( i = 0; args[i] != NULL; i += 1 ) {
	cerr << args[i] << " ";
    } // for
    cerr << endl;
#endif // __U_DEBUG_H__

    execvp( args[0], (char * const *)args );		// should not return
    perror( "uC++ Translator error: cpp level, execvp" );
    exit( EXIT_FAILURE );				// tell gcc not to go any further
} // Stage2


int main( const int argc, const char * const argv[], const char * const env[] ) {
#ifdef __U_DEBUG_H__
    for ( int i = 0; env[i] != NULL; i += 1 ) {
	cerr << env[i] << endl;
    } // for
#endif // __U_DEBUG_H__

    string arg = argv[1];

    // Currently, stage 1 starts with flag -E and stage 2 with flag -fpreprocessed.

    if ( arg == "-E" ) {
#ifdef __U_DEBUG_H__
	cerr << "Stage1" << endl;
#endif // __U_DEBUG_H__
	Stage1( argc, argv );
    } else if ( arg == "-fpreprocessed" ) {
#ifdef __U_DEBUG_H__
	cerr << "Stage2" << endl;
#endif // __U_DEBUG_H__
	Stage2( argc, argv );
    } else {
	fprintf( stderr, "Usage: %s input-file [output-file] [options]\n", argv[0] );
	exit( EXIT_FAILURE );
    } // if
} // main


// Local Variables: //
// compile-command: "make install" //
// End: //
