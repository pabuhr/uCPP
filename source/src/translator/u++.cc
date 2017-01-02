//                              -*- Mode: C++ -*-
//
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr and Nikita Borisov 1995
//
// u++.cc --
//
// Author           : Nikita Borisov
// Created On       : Tue Apr 28 15:26:27 1992
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Dec 25 10:31:35 2016
// Update Count     : 925
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
#include <fstream>					// ifstream
#include <cstdio>					// perror
#include <cstdlib>					// getenv, putenv
#include <cstring>					// strcmp, strlen
#include <string>					// STL version
#include <unistd.h>					// execvp

using std::ifstream;
using std::cerr;
using std::endl;
using std::string;


//#define __U_DEBUG_H__

#define STRINGIFY(s) #s
#define VSTRINGIFY(s) STRINGIFY(s)


bool prefix( string arg, string pre ) {
    return arg.substr( 0, pre.size() ) == pre;
} // prefix


void shuffle( const char *args[], int S, int E, int N ) {
    // S & E index 1 passed the end so adjust with -1
#ifdef __U_DEBUG_H__
    cerr << "shuffle:" << S << " " << E << " " << N << endl;
#endif // __U_DEBUG_H__
    for ( int j = E-1 + N; j > S-1 + N; j -=1 ) {
#ifdef __U_DEBUG_H__
	cerr << "\t" << j << " " << j-N << endl;
#endif // __U_DEBUG_H__
	args[j] = args[j-N];
    } // for
} // shuffle


int main( int argc, char *argv[] ) {
    string Version( VERSION );				// current version number from CONFIG
    string Major( "0" ), Minor( "0" ), Patch( "0" );	// default version numbers
    int posn1 = Version.find( "." );			// find the divider between major and minor version numbers
    if ( posn1 == -1 ) {				// not there ?
	Major = Version;
    } else {
	Major = Version.substr( 0, posn1 );
	int posn2 = Version.find( ".", posn1 + 1 );	// find the divider between minor and patch numbers
	if ( posn2 == -1 ) {				// not there ?
	    Minor = Version.substr( posn1 );
	} else {
	    Minor = Version.substr( posn1 + 1, posn2 - posn1 - 1 );
	    Patch = Version.substr( posn2 + 1 );
	} // if
    } // if

    string installincdir( INSTALLINCDIR );		// fixed location of include files
    string installlibdir( INSTALLLIBDIR );		// fixed location of the cc1 and cfa-cpp commands

    string tvendor( TVENDOR );
    string tos( TOS );
    string tcpu( TCPU );

    string Multi( MULTI );

    string heading;					// banner printed at start of cfa compilation
    string arg;						// current command-line argument during command-line parsing
    string Bprefix;					// path where g++ looks for compiler command steps
    string langstd;					// language standard

    string compiler_path( CCAPP );			// path/name of C compiler
    string compiler_name;				// name of C compiler

    string cpp11;					// C++11 version

    bool nonoptarg = false;				// indicates non-option argument specified
    bool link = true;					// linking as well as compiling
    bool verbose = false;				// -v flag
    bool quiet = false;					// -quiet flag
    bool debug = true;					// -debug flag
    bool multi = false;					// -multi flag
    bool upp_flag = false;				// -U++ flag
    bool cpp_flag = false;				// -E or -M flag, preprocessor only
    bool Yield = false;					// -yield flag (name "yield" already taken)
    bool verify = false;				// -verify flag
    bool profile = false;				// -profile flag
    bool exact = true;					// profile type: exact | statistical
    bool debugging = false;				// -g flag
    bool openmp = false;				// -openmp flag
    bool nouinc = false;				// -no-u++-include: avoid "inc" directory

    const char *args[argc + 100];			// u++ command line values, plus some space for additional flags
    int sargs = 1;					// starting location for arguments in args list
    int nargs = sargs;					// number of arguments in args list; 0 => command name

    const char *libs[argc + 20];			// non-user libraries must come separately, plus some added libraries and flags
    int nlibs = 0;

    string tmppath;
    string mvdpath;
    string bfdincdir;
    string motifincdir;
    string motiflibdir;
    string uxincdir;
    string uxlibdir;
    string mvdincdir;
    string mvdlibdir;
    string useperfmonstr;
    string perfmonincdir;
    string perfmonlibdir;
    string unwindincdir;
    string unwindlibdir;
    string token;
    string uAlloc;

#ifdef __U_DEBUG_H__
    cerr << "u++:" << endl;
#endif // __U_DEBUG_H__

    // process command-line arguments

    for ( int i = 1; i < argc; i += 1 ) {
#ifdef __U_DEBUG_H__
	cerr << "argv[" << i << "]:\"" << argv[i] << "\"" << endl;
#endif // __U_DEBUG_H__
	arg = argv[i];					// convert to string value
#ifdef __U_DEBUG_H__
	cerr << "arg:\"" << arg << "\"" << endl;
#endif // __U_DEBUG_H__
	if ( prefix( arg, "-" ) ) {
	    // pass through arguments

	    if ( arg == "-Xlinker" || arg == "-o" ) {
		args[nargs] = argv[i];			// pass the argument along
		nargs += 1;
		i += 1;
		if ( i == argc ) continue;		// next argument available ?
		args[nargs] = argv[i];			// pass the argument along
		nargs += 1;

	    // uC++ specific arguments

	    } else if ( arg == "-U++" ) {
		upp_flag = true;			// strip the -U++ flag
		link = false;
		args[nargs] = "-E";			// replace the argument with -E
		nargs += 1;
	    } else if ( arg == "-multi" ) {
		multi = true;				// strip the multi flag
	    } else if ( arg == "-nomulti" ) {
		multi = false;				// strip the nomulti flag
	    } else if ( arg == "-debug" ) {
		debug = true;				// strip the debug flag
	    } else if ( arg == "-nodebug" ) {
		debug = false;				// strip the nodebug flag
	    } else if ( arg == "-quiet" ) {
		quiet = true;				// strip the quiet flag
	    } else if ( arg == "-noquiet" ) {
		quiet = false;				// strip the noquiet flag
	    } else if ( arg == "-yield" ) {
		Yield = true;				// strip the yield flag
	    } else if ( arg == "-noyield" ) {
		Yield = false;				// strip the noyield flag
	    } else if ( arg == "-verify" ) {
		verify = true;				// strip the verify flag
	    } else if ( arg == "-noverify" ) {
		verify = false;				// strip the noverify flag
	    } else if ( arg == "-profile" ) {
                profile = true;                         // strip the profile flag
		if ( i + 1 < argc ) {			// check next argument, if available
		    if ( strcmp( argv[i + 1], "exact" ) == 0 ) { // default ?
			i += 1;				// skip argument
		    } else if ( strcmp( argv[i + 1], "statistical" ) == 0 ) {
			exact = false;
			i += 1;				// skip argument
		    } // if
		} // if
            } else if ( arg == "-noprofile" ) {
                profile = false;                        // strip the noprofile flag
	    } else if ( arg == "-compiler" ) {
		// use the user specified compiler
		i += 1;
		if ( i == argc ) continue;		// next argument available ?
		compiler_path = argv[i];
		if ( putenv( (char *)( *new string( string( "__U_COMPILER__=" ) + argv[i]) ).c_str() ) != 0 ) {
		    cerr << argv[0] << " error, cannot set environment variable." << endl;
		    exit( EXIT_FAILURE );
		} // if
	    } else if ( arg == "-no-u++-include" ) {
		nouinc = true;

	    // C++ specific arguments

	    } else if ( arg == "-v" ) {
		verbose = true;				// verbosity required
		args[nargs] = argv[i];			// pass the argument along
		nargs += 1;
	    } else if ( arg == "-g" ) {
		debugging = true;			// symbolic debugging required
		args[nargs] = argv[i];			// pass the argument along
		nargs += 1;
	    } else if ( prefix( arg, "-std=" ) || prefix( arg, "--std=" ) ) {
		string langstd = arg.substr( arg[1] == '-' ? 6 : 5 ); // strip the -std= flag
		if ( langstd == "c++0x" || langstd == "gnu++0x" ||
		     langstd == "c++11" || langstd == "gnu++11" ||
		     langstd == "c++14" || langstd == "gnu++14" ||
		     langstd == "c++17" || langstd == "gnu++17" ||
		     langstd == "c++1y" || langstd == "gnu++1y"
		    ) {
		    cpp11 = langstd;			// unsure if other values are valid
		} // if
		args[nargs] = argv[i];			// pass the argument along
		nargs += 1;
	    } else if ( prefix( arg, "-B" ) ) {
		Bprefix = arg.substr(2);		// strip the -B flag
		args[nargs] = ( *new string( string("-D__U_GCC_BPREFIX__=") + Bprefix ) ).c_str();
		nargs += 1;
	    } else if ( prefix( arg, "-b" ) ) {
		if ( arg.length() == 2 ) {		// separate argument ?
		    i += 1;
		    if ( i == argc ) continue;		// next argument available ?
		    arg += argv[i];			// concatenate argument
		} // if
		// later versions of gcc require the -b option to appear at the start of the command line
		shuffle( args, sargs, nargs, 1 );	// make room at front of argument list
		args[sargs] = ( *new string( arg ) ).c_str(); // pass the argument along
		if ( putenv( (char *)( *new string( string( "__U_GCC_MACHINE__=" ) + arg ) ).c_str() ) != 0 ) {
		    cerr << argv[0] << " error, cannot set environment variable." << endl;
		    exit( EXIT_FAILURE );
		} // if
		sargs += 1;
		nargs += 1;
	    } else if ( prefix( arg, "-V" ) ) {
		if ( arg.length() == 2 ) {		// separate argument ?
		    i += 1;
		    if ( i == argc ) continue;		// next argument available ?
		    arg += argv[i];			// concatenate argument
		} // if
		// later versions of gcc require the -V option to appear at the start of the command line
		shuffle( args, sargs, nargs, 1 );	// make room at front of argument list
		args[sargs] = ( *new string( arg ) ).c_str(); // pass the argument along
		if ( putenv( (char *)( *new string( string( "__U_GCC_VERSION__=" ) + arg ) ).c_str() ) != 0 ) {
		    cerr << argv[0] << " error, cannot set environment variable." << endl;
		    exit( EXIT_FAILURE );
		} // if
		sargs += 1;
		nargs += 1;
	    } else if ( arg == "-c" || arg == "-S" || arg == "-E" || arg == "-M" || arg == "-MM" ) {
		args[nargs] = argv[i];			// pass the argument along
		nargs += 1;
		if ( arg == "-E" || arg == "-M" || arg == "-MM" ) {
		    cpp_flag = true;			// cpp only
		} // if
		link = false;                           // no linkage required
	    } else if ( prefix( arg, "-uAlloc" ) ) {
		// non-default memory allocator
		uAlloc = &argv[i][1];
	    } else if ( arg[1] == 'l' ) {
		// if the user specifies a library, load it after user code
		libs[nlibs] = argv[i];
		nlibs += 1;
	    } else if ( arg == "-openmp" ) {
		openmp = true;				// openmp mode
		args[nargs] = argv[i];			// pass the argument along
		nargs += 1;
	    } else {
		// concatenate any other arguments
		args[nargs] = argv[i];
		nargs += 1;
	    } // if
	} else {
	    // concatenate other arguments
	    args[nargs] = argv[i];
	    nargs += 1;
	    nonoptarg = true;
	} // if
    } // for

#ifdef __U_DEBUG_H__
    cerr << "args:";
    for ( int i = 1; i < nargs; i += 1 ) {
	cerr << " " << args[i];
    } // for
    cerr << endl;
#endif // __U_DEBUG_H__

    if ( cpp_flag && upp_flag ) {
	cerr << argv[0] << " error, cannot use -E and -U++ flags together." << endl;
	exit( EXIT_FAILURE );
    } // if

    string d;
    if ( debug ) {
	d = "-d";
    } // if

    string m;
    if ( multi ) {
	if ( Multi == "FALSE" ) {			// system support multiprocessor ?
	    cerr << argv[0] << ": Warning -multi flag not support on this system." << endl;
	} else {
	    m = "-m";
	} // if
    } // if

    // profiling

    if ( profile ) {					// read MVD configuration information needed for compilation and/or linking.
	char *mvdpathname = getenv( "MVDPATH" );	// get MVDPATH environment variable

	if ( mvdpathname == nullptr ) {
	    cerr << argv[0] << ": Warning environment variable MVDPATH not set. Profiling disabled." << endl;
	    profile = false;
	} else {
	    mvdpath = mvdpathname;			// make string
	    if ( mvdpathname[strlen( mvdpathname ) - 1] != '/' ) { // trailing slash ?
		mvdpath += "/";				// add slash
	    } // if

	    // Define lib and include directories
	    uxincdir = mvdpath + "X11R6/include";
	    uxlibdir = mvdpath + "X11R6/lib";

	    // Read Motif and MVD lib and include directories from the MVD CONFIG file

	    string configFilename = mvdpath + "CONFIG";
	    ifstream configFile( configFilename.c_str() );

	    if ( ! configFile.good() ) {
		cerr << argv[0] << ": Warning could not open file \"" << configFilename << "\". Profiling disabled." << endl;
		profile = false;
	    } else {
		const int dirnum = 11;
		struct {
		    const char *dirkind;
		    int used;
		    string *dirname;
		} dirs[dirnum] = {
		    { "INSTALLINCDIR", 1, &mvdincdir },
		    { "INSTALLLIBDIR", 1, &mvdlibdir },
		    { "BFDINCLUDEDIR", 1, &bfdincdir },
		    { "MOTIFINCLUDEDIR", 1, &motifincdir },
		    { "MOTIFLIBDIR", 1, &motiflibdir },
		    { "PERFMON", 1, &useperfmonstr },
		    { "PFMINCLUDEDIR", 1, &perfmonincdir },
		    { "PFMLIBDIR", 1, &perfmonlibdir },
		    { "UNWINDINCLUDEDIR", 1, &unwindincdir },
		    { "UNWINDLIBDIR", 1, &unwindlibdir },
		    { "TMPDIR", 1, &tmppath },
		};
		string dirkind, equal, dirname;
		int cnt, i;
		int numOfDir = 0;

		for ( cnt = 0 ; cnt < dirnum; cnt += 1 ) { // names can appear in any order
		    for ( ;; ) {
			configFile >> dirkind;
		  if ( configFile.eof() || configFile.fail() ) goto fini;
			for ( i = 0; i < dirnum && dirkind != dirs[i].dirkind; i += 1 ) {} // linear search
		      if ( i < dirnum ) break;		// found a line to be parsed
		    } // for
		    configFile >> equal;
		  if ( configFile.eof() || configFile.fail() || equal != "=" ) break;
		    getline( configFile, dirname );	// could be empty
		  if ( configFile.eof() || ! configFile.good() ) break;
		    int p = dirname.find_first_not_of( " " ); // find position of 1st blank character
		    if ( p == -1 ) p = dirname.length(); // any characters left ?
		    dirname = dirname.substr( p );	// remove leading blanks

		    numOfDir += dirs[i].used;		// handle repeats
		    dirs[i].used = 0;
		    *dirs[i].dirname = dirname;
#ifdef __U_DEBUG_H__
		    cerr << dirkind << equal << dirname << endl;
#endif // __U_DEBUG_H__
		} // for
	      fini:
		if ( numOfDir != dirnum ) {
		    profile = false;
		    cerr << argv[0] << ": Warning file \"" << configFilename << "\" corrupt.  Profiling disabled." << endl;
		} // if
	    } // if
	} // if
    } // if

    if ( link ) {
	// shift arguments to make room for special libraries

	int pargs = 0;
	if ( profile ) {
	    pargs += 7;					// N profiler arguments added at beginning
	} // if
	shuffle( args, sargs, nargs, pargs );
	nargs += pargs;

	if ( profile ) {
	    // link the profiling library before the user code
	    args[sargs] = "-u";
	    sargs += 1;
	    args[sargs] = "U_SMEXACTMENUWD";		// force profiler start-up widgets to be loaded
	    sargs += 1;
	    args[sargs] = "-u";
	    sargs += 1;
	    args[sargs] = "U_SMSTATSMENUWD";		// force profiler start-up widgets to be loaded
	    sargs += 1;
	    args[sargs] = "-u";
	    sargs += 1;
	    args[sargs] = "U_SMOTHERMENUWD";		// force profiler start-up widgets to be loaded
	    sargs += 1;
	    // SKULLDUGGERY: Put the profiler library before the user code to force the linker to include the
	    // no-profiled versions of compiled inlined routines from uC++.h.
	    args[sargs] = ( *new string( mvdlibdir + "/uProfile"  + m + d + ".a" ) ).c_str();
	    sargs += 1;
	    // SKULLDUGGERY: Put the profiler library after the user code to force the linker to include the
	    // -finstrument-functions, if there is any reference to them in the user code.
	    args[nargs] = ( *new string( mvdlibdir + "/uProfile"  + m + d + ".a" ) ).c_str();
	    nargs += 1;
	    if ( ! debugging ) {			// add -g if not specified
		args[nargs] = "-g";
		nargs += 1;
	    } // if
	} // if

	// override uDefaultProcessors for OpenMP -- must come before uKernel

	if ( openmp ) {
	    args[nargs] = ( *new string( installlibdir + "/uDefaultProcessors-OpenMP.o" ) ).c_str();
	    nargs += 1;
	} // if

	if ( uAlloc != "" ) {
 	    args[nargs] = "-u";
 	    nargs += 1;
 	    args[nargs] = "malloc";			// force heap to be loaded
 	    nargs += 1;
 	    args[nargs] = "-u";
 	    nargs += 1;
 	    args[nargs] = "_ZN12uHeapControl11prepareTaskEP9uBaseTask";
 	    nargs += 1;
	    args[nargs] = ( *new string( installlibdir + "/" + uAlloc + m + d + ".a" ) ).c_str();
	    nargs += 1;
	} // if

	// link with the correct version of the kernel module

	args[nargs] = ( *new string( installlibdir + "/uKernel" + m + d + ".a" ) ).c_str();
	nargs += 1;
	args[nargs] = ( *new string( installlibdir + "/uScheduler" + m + d + ".a" ) ).c_str();
	nargs += 1;

	// link with the correct version of the local debugger module

	args[nargs] = ( *new string( installlibdir + "/uLocalDebugger" + m + "-d.a" ) ).c_str();
	nargs += 1;

	// link with the correct version of the library module

	args[nargs] = ( *new string( installlibdir + "/uLibrary" + m + d + ".a" ) ).c_str();
	nargs += 1;

	// link with the correct version of the profiler module

	args[nargs] = ( *new string( installlibdir + "/uProfilerFunctionPointers" + ".o" ) ).c_str();
	nargs += 1;

	// any machine specific libraries

	if ( tos != "freebsd" ) {			// not on freebsd at all
	    libs[nlibs] = "-ldl";			// calls to dlsym/dlerror
	    nlibs += 1;
	} // if

	if ( profile ) {
	    args[nargs] = ( *new string( string("-L") + mvdlibdir ) ).c_str();
	    nargs += 1;
	    args[nargs] = ( *new string( string("-L") + uxlibdir ) ).c_str();
	    nargs += 1;
	    if ( motiflibdir.length() != 0 ) {
		args[nargs] = ( *new string( string("-L") + motiflibdir ) ).c_str();
		nargs += 1;
	    } // if
	    args[nargs] = "-L/usr/X11R6/lib";
	    nargs += 1;
	    args[nargs] = ( *new string( string("-Wl,-R,") + uxlibdir + ( motiflibdir.length() != 0 ? string(":") + motiflibdir : "" ) ) ).c_str();
	    nargs += 1;
	    libs[nlibs] = "-lXm";
	    nlibs += 1;
	    libs[nlibs] = "-lX11";
	    nlibs += 1;
	    libs[nlibs] = "-lXt";
	    nlibs += 1;
	    libs[nlibs] = "-lSM";
	    nlibs += 1;
	    libs[nlibs] = "-lICE";
//	    nlibs += 1;
//	    libs[nlibs] = "-lXpm";
	    nlibs += 1;
	    libs[nlibs] = "-lXext";
	    nlibs += 1;
	    libs[nlibs] = ( *new string( string( "-luX" ) + m + d ) ).c_str();
	    nlibs += 1;
	    libs[nlibs] = "-lm";
	    nlibs += 1;
	    libs[nlibs] = "-lbfd";
	    nlibs += 1;
	    libs[nlibs] = "-liberty";
	    nlibs += 1;

	    if ( tos == "solaris" ) {			// link in performance counter
                libs[nlibs] = "-lcpc";
                nlibs += 1;
	    } // if
	    if ( perfmonlibdir.length() != 0 ) {
		args[nargs] = ( *new string( string( "-L" ) + perfmonlibdir ) ).c_str();
		nargs += 1;
		args[nargs] = ( *new string( string("-Wl,-R,") + perfmonlibdir ) ).c_str();
		nargs += 1;
	    } // if
	    if ( unwindlibdir.length() != 0 ) {
		args[nargs] = ( *new string( string( "-L" ) + unwindlibdir ) ).c_str();
		nargs += 1;
		args[nargs] = ( *new string( string("-Wl,-R,") + unwindlibdir ) ).c_str();
		nargs += 1;
	    } // if
	    if ( useperfmonstr == "PERFMON" ) {		// link in performance counter
                libs[nlibs] = "-lpfm";
                nlibs += 1;
	    } else if ( useperfmonstr == "PERFMON3" ) {
                libs[nlibs] = "-lpfm3";
                nlibs += 1;
	    } else if ( useperfmonstr == "PERFCTR" ) {
                libs[nlibs] = "-lperfctr";
                nlibs += 1;
	    } // if
	    if ( tos == "linux" && ( tcpu == "ia64" || tcpu == "x86_64" ) ) {
                libs[nlibs] = "-lunwind";		// link in libunwind for backtraces
                nlibs += 1;
	    } // if
	} // if

	if ( tos == "solaris" ) {
	    libs[nlibs] = "-lnsl";
	    nlibs += 1;
	    libs[nlibs] = "-lsocket";
	    nlibs += 1;
	    libs[nlibs] = "-lsendfile";
	    nlibs += 1;
	    libs[nlibs] = "-lrt";			// sched_yield
	    nlibs += 1;
	} // if

	if ( multi ) {
	    libs[nlibs] = "-lpthread";
	    nlibs += 1;
	} // if
    } // if

    // The u++ translator is used to build the kernel and library support code to allow uC++ keywords, like _Coroutine,
    // _Task, _Mutex, etc. However, the translator does not need to make available the uC++ includes during kernel build
    // because the include directory (inc) is being created, so these directories and special includes are turned off
    // (see src/MakeTools). Kernel build is the only time this flag should be used.
    //
    // Note, when testing using -no-u++-include, the "inc" file is not present so special include files to adjust text
    // do not occur. Hence, other errors may occur. See the "library" directory special include files.

    if ( ! nouinc ) {
	// add the directory that contains the include files to the list of arguments after any user specified include
	// directives

	args[nargs] = ( *new string( string("-I") + installincdir ) ).c_str();
	nargs += 1;

	// automagically add uC++.h as the first include to each translation unit so users do not have to remember to do
	// this

	args[nargs] = "-include";
	nargs += 1;
	args[nargs] = ( *new string( installincdir + string("/uC++.h") ) ).c_str();
	nargs += 1;
    } // if

    if ( profile ) {
	args[nargs] = ( *new string( string( "-I" ) + mvdincdir ) ).c_str();
	nargs += 1;
	args[nargs] = ( *new string( string( "-I" ) + uxincdir ) ).c_str();
	nargs += 1;
	if ( motifincdir.length() != 0 ) {
	    args[nargs] = ( *new string( string( "-I" ) + motifincdir ) ).c_str();
	    nargs += 1;
	} // if
	if ( bfdincdir.length() != 0 ) {
	    args[nargs] = ( *new string( string( "-I" ) + bfdincdir ) ).c_str();
	    nargs += 1;
	} // if
    } // if

    // add the correct set of flags based on the type of compile this is

    args[nargs] = ( *new string( string("-D__U_CPLUSPLUS__=") + Major ) ).c_str();
    nargs += 1;
    args[nargs] = ( *new string( string("-D__U_CPLUSPLUS_MINOR__=") + Minor ) ).c_str();
    nargs += 1;
    args[nargs] = ( *new string( string("-D__U_CPLUSPLUS_PATCH__=") + Patch ) ).c_str();
    nargs += 1;

    if ( cpp_flag ) {
	args[nargs] = "-D__U_CPP__";
	nargs += 1;
    } // if

    if ( upp_flag ) {
	args[nargs] = "-D__U_UPP__";
	nargs += 1;
    } // if

    if ( multi ) {
	args[nargs] = "-D__U_MULTI__";
	nargs += 1;
	heading += " (multiple processor)";
    } else {
	heading += " (single processor)";
    } // if

    if ( debug ) {
	heading += " (debug)";
	args[nargs] = "-D__U_DEBUG__";
	nargs += 1;
    } else {
	heading += " (no debug)";
    } // if

    if ( Yield ) {
	heading += " (yield)";
	args[nargs] = "-D__U_YIELD__";
	nargs += 1;
    } else {
	heading += " (no yield)";
    } // if

    if ( verify ) {
	heading += " (verify)";
	args[nargs] = "-D__U_VERIFY__";
	nargs += 1;
    } else {
	heading += " (no verify)";
    } // if

    if ( profile ) {
        heading += " (profile)";
	if ( exact ) {
	    args[nargs] = ( *new string( "-finstrument-functions" ) ).c_str();
	    nargs += 1;
	} // if
        args[nargs] = "-D__U_PROFILE__";
        nargs += 1;
    } else {
        heading += " (no profile)";
    } // if

    if ( openmp ) {
	args[nargs] = "-D__U_OPENMP__";
	nargs += 1;
    } // if

    args[nargs] = "-D__U_MAXENTRYBITS__=" VSTRINGIFY(__U_MAXENTRYBITS__); // has underscores because it is used to build translator
    nargs += 1;

    args[nargs] = "-D__U_WORDSIZE__=" VSTRINGIFY(WORDSIZE);
    nargs += 1;

#if defined( STATISTICS )				// Kernel Statistics ?
    args[nargs] = "-D__U_STATISTICS__";
    nargs += 1;
#endif // STATISTICS

#if defined( AFFINITY )					// Thread Local Storage ?
    args[nargs] = "-D__U_AFFINITY__";
    nargs += 1;
#endif // AFFINITY

#if defined( BROKEN_CANCEL )				// TEMPORARY: old glibc has pthread_testcancel as throw()
    args[nargs] = "-D__U_BROKEN_CANCEL__";
    nargs += 1;
#endif // BROKEN_CANCEL

    if ( Bprefix.length() == 0 ) {
	Bprefix = installlibdir;
	args[nargs] = ( *new string( string("-D__U_GCC_BPREFIX__=") + Bprefix ) ).c_str();
	nargs += 1;
    } // if

    if ( tcpu == "sparc" && tos == "solaris" && string(VSTRINGIFY(WORDSIZE)) == "32" ) {
	// tell assembler it's ok to use cas for 32-bit architecture; 64-bit architecture => -xarch=v9, which allows cas
	args[nargs] = "-Wa,-xarch=v8plusa";
	nargs += 1;
    } // if

    args[nargs] = "-Xlinker";				// used by backtrace
    nargs += 1;
    args[nargs] = "-export-dynamic";
    nargs += 1;

    // execute the compilation command

    args[0] = compiler_path.c_str();			// set compiler command for exec
    // find actual name of the compiler independent of the path to it
    int p = compiler_path.find_last_of( '/' );		// scan r -> l for first '/'
    if ( p == -1 ) {
	compiler_name = compiler_path;
    } else {
	compiler_name = *new string( compiler_path.substr( p + 1 ) );
    } // if

    if ( prefix( compiler_name, "g++" ) ) {		// allow suffix on g++ name
	if ( tcpu == "ia64" ) {
	    args[nargs] = "-fno-optimize-sibling-calls"; // TEMPORARY: gcc 3 code gen problem on ia64
	    nargs += 1;
//	} else if ( tcpu == "i386" ) {
//	    args[nargs] = "-march=i586";		// minimum architecture level for __sync_fetch_and_add
//	    nargs += 1;
	} else if ( tcpu == "sparc" ) {
	    args[nargs] = "-mcpu=v9";			// minimum architecture level for __sync_fetch_and_add
	    nargs += 1;
	} // if
	// minimum c++0x
	if ( cpp11.length() == 0 ) {
	    args[nargs] = ( *new string( string("-std=") + CPP11 ) ).c_str(); // default
	} else {
	    args[nargs] = ( *new string( string("-std=") + cpp11 ) ).c_str(); // user supplied
	} // if
	nargs += 1;
	args[nargs] = ( *new string( string("-D__U_STD_CPP11__") ) ).c_str();
	nargs += 1;
	args[nargs] = "-no-integrated-cpp";
	nargs += 1;
	args[nargs] = ( *new string( string("-B") + Bprefix + "/" ) ).c_str();
	nargs += 1;
    } else {
	cerr << argv[0] << " error, compiler \"" << compiler_name << "\" unsupported." << endl;
	exit( EXIT_FAILURE );
    } // if

    // Add the uC++ definitions of vendor, cpu and os names to the compilation command.

    args[nargs] = ( *new string( string("-D__") + TVENDOR + "__" ) ).c_str();
    nargs += 1;
    args[nargs] = ( *new string( string("-D__") + TCPU + "__" ) ).c_str();
    nargs += 1;
    args[nargs] = ( *new string( string("-D__") + TOS + "__" ) ).c_str();
    nargs += 1;

    for ( int i = 0; i < nlibs; i += 1 ) {		// copy non-user libraries after all user libraries
	args[nargs] = libs[i];
	nargs += 1;
    } // for

    args[nargs] = nullptr;				// terminate with null

#ifdef __U_DEBUG_H__
    cerr << "nargs: " << nargs << endl;
    cerr << "args:" << endl;
    for ( int i = 0; args[i] != nullptr; i += 1 ) {
	cerr << " \"" << args[i] << "\"" << endl;
    } // for
#endif // __U_DEBUG_H__

    if ( ! quiet ) {
	cerr << "uC++ " << "Version " << Version << heading << endl;
    } // if

    if ( verbose ) {
	if ( argc == 2 ) exit( EXIT_SUCCESS );		// if only the -v flag is specified, do not invoke g++

	for ( int i = 0; args[i] != nullptr; i += 1 ) {
	    cerr << args[i] << " ";
	} // for
	cerr << endl;
    } // if

    if ( ! nonoptarg ) {
	cerr << argv[0] << " error, no input files" << endl;
	exit( EXIT_FAILURE );
    } // if

    // execute the command and return the result

    execvp( args[0], (char *const *)args );		// should not return
    perror( "uC++ Translator error: u++ level, execvp" );
    exit( EXIT_FAILURE );
} // main

// Local Variables: //
// compile-command: "make install" //
// End: //
