//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 2014
// 
// uCobegin.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sat Dec 27 18:31:33 2014
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Dec 28 16:04:52 2014
// Update Count     : 13
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


#ifndef __U_COBEGIN_H__
#define __U_COBEGIN_H__


#if __cplusplus > 201103L

#include <functional>
#include <memory>

#pragma __U_NOT_USER_CODE__

// COBEGIN

#define COBEGIN uCobegin( {
#define COEND } );
#define BEGIN [&]( unsigned int uLid ) {
#define END } ,

void uCobegin( std::initializer_list< std::function< void( unsigned int ) >> funcs ) {
    unsigned int uLid = 0;
    _Task Runner {
	typedef std::function<void( unsigned int )> Func; // function type
	unsigned int parm;				// local thread lid
	Func f;						// function to run for each lid

	void main() { f( parm ); }
      public:
	Runner( unsigned int parm, Func f ) : parm( parm ), f( f ) {}
    }; // Runner

    const unsigned int size = funcs.size();
    Runner **runners = new Runner *[size];		// do not use up task stack

    for ( auto f : funcs ) { runners[uLid] = new Runner( uLid, f ); uLid += 1; }
    for ( uLid = 0; uLid < size; uLid += 1 ) delete runners[uLid];
    delete [] runners;
} // uCobegin

// COFOR

#define COFOR( lidname, low, high, body ) uCofor( low, high, [&]( unsigned int lidname ){ body } );

template<typename Low, typename High>			// allow bounds to have different types (needed for constants)
void uCofor( Low low, High high, std::function<void ( unsigned int )> f ) {
    unsigned int lid = 0;
    _Task Runner {
	typedef std::function<void( unsigned int )> Func; // function type
	unsigned int parm;				// local thread lid
	Func f;						// function to run for each lid

	void main() { f( parm ); }
      public:
	Runner( unsigned int parm, Func f ) : parm( parm ), f( f ) {}
    }; // Runner

    assert( 0 <= high - low );
    const decltype(lid) size = high - low;
    Runner **runners = new Runner *[size];		// do not use up task stack

    for ( unsigned int lid = 0; lid < size; lid += 1 ) runners[lid] = new Runner( lid + low, f );
    for ( unsigned int lid = 0; lid < size; lid += 1 ) delete runners[lid];
    delete [] runners;
} // uCofor

// START/WAIT

template<typename T, typename... Args>
_Task uWaitRunner {
    T rVal;
    std::function<T(Args...)> f;
    std::tuple<Args...> args;

    template<std::size_t... I>
    void call( std::index_sequence<I...> ) {
	rVal = f(std::get<I>(args)...);
    } // uWaitRunner::call

    void main() {
	call( std::index_sequence_for<Args...>{} );
    } // uWaitRunner::main
  public:
    uWaitRunner( std::function<T(Args...)> f, Args&&... args ) : f( f ), args( std::forward_as_tuple( args... ) ) {}
    T join() { return rVal; }
}; // uWaitRunner

template<typename... Args>
_Task uWaitRunner<void, Args...> {
    std::function<void(Args...)> f;
    std::tuple<Args...> args;

    template<std::size_t... I>
    void call( std::index_sequence<I...> ) {
	f( std::get<I>(args)... );
    } // uWaitRunner::call

    void main() {
	call( std::index_sequence_for<Args...>{} );
    } // uWaitRunner::main
  public:
    uWaitRunner( std::function<void(Args...)> f, Args&&... args ) : f( f ), args( std::forward_as_tuple( args... ) ) {}
    void join() {}
}; // uWaitRunner

template<typename F, typename... Args>
auto START( F f, Args&&... args ) -> std::unique_ptr<uWaitRunner<decltype( f( args... ) ), Args...>> {
    return std::make_unique<uWaitRunner<decltype( f(args...) ), Args...>>( f, std::forward<Args>(args)... );
} // START

#define WAIT( handle ) handle->join()

#else
    #error requires C++14 (-std=c++1y)
#endif


#pragma __U_USER_CODE__

#endif // __U_COBEGIN_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
