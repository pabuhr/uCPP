//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2009
// 
// assert.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Dec 10 20:40:07 2009
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Aug 11 01:24:41 2012
// Update Count     : 43
// 

// This include file is not idempotent, so there is no guard.

#ifdef NDEBUG

#   define assert( expr ) ((void)0)

#else

#   include <stdlib.h>					// abort
#   include <unistd.h>					// write

#   define __STRINGIFY__(str) #str
#   define __VSTRINGIFY__(str) __STRINGIFY__(str)

#   define assert( expr ) \
	if ( ! ( expr ) ) { \
	    int retcode __attribute__(( unused ));			\
	    retcode = ::write( STDERR_FILENO, __FILE__ ":" __VSTRINGIFY__(__LINE__) ": ", \
			       sizeof(__FILE__ ":" __VSTRINGIFY__(__LINE__) ": ") - 1 ); \
	    retcode = ::write( STDERR_FILENO, __PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 1 ); \
	    retcode = ::write( STDERR_FILENO, ": Assertion \"" __VSTRINGIFY__(expr) "\" failed.\n", \
			       sizeof(": Assertion \"" __VSTRINGIFY__(expr) "\" failed.\n") - 1 ); \
	    abort(); \
	}

#endif // NDEBUG


// Local Variables: //
// compile-command: "make install" //
// End: //
