//                              -*- Mode: C++ -*- 
// 
// Copyright (C) Peter A. Buhr 2014
// 
// Futures.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Tue Mar  4 23:12:12 2014
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon May 11 23:03:58 2015
// Update Count     : 63
// 

#include <uFuture.h>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

enum { N = 2, NoOfTime = 1 };

_Task Worker {
    unsigned int id;
    Future_ISM<int> &f;

    void main() {
	for ( unsigned int i = 0; i < NoOfTime; i += 1 ) {
	    std::osacquire( std::cerr ) << id << " " << i << endl;
	    f.delivery( id );
	    yield( 300 );
	    while ( f.available() ) {			// busy wait
		yield( rand() % 3 );
	    } // while
	} // for
    } // Worker::main
  public:
    Worker( unsigned int id, Future_ISM<int> &f ) : id( id ), f( f ) {}
}; // Worker

void uMain::main() {
    Future_ISM<int> f[N];
    Worker *workers[N];
    //uProcessor p[N - 1];
    bool check;

    for ( unsigned int i = 0; i < N; i += 1 ) {
	workers[i] = new Worker( i, f[i] );
    } // for
#if 0
    for ( unsigned int i = 0; i < N * NoOfTime; i += 1 ) {
	check = true;

	_Select( f[0] || f[1] && f[2] ) {
	    assert( check );
	    check = false;
	    yield( rand() % 3 );
	    if ( f[0].available() ) {
		f[0].reset();
	    } else if ( f[1].available() ) {
		f[1].reset();
	    } else if ( f[2].available() ) {
		f[2].reset();
	    } else {
		abort();
	    } // if
	} // _Select
    } // for
#endif
#if 1
    for ( unsigned int i = 0; i < 2 * NoOfTime; i += 1 ) {
	check = true;

//	f[0].delivery( i );
//	f[1].delivery( i );
	_Select( f[0] || f[1] );


	_Select( f[0] ) {
	    std::osacquire( std::cerr ) << "f0" << endl;
	    assert( check );
	    check = false;
	    yield( rand() % 3 );
	    f[0].reset();
	    std::osacquire( std::cerr ) << "f0 available " << f[0].available() << endl;
	} or _Select( f[1] ) {
	    std::osacquire( std::cerr ) << "f1" << endl;
	    assert( check );
	    check = false;
	    yield( rand() % 3 );
	    f[1].reset();
	    std::osacquire( std::cerr ) << "f1 available " << f[1].available() << endl;
//	} and _Select( f[2] ) {
//	    assert( check );
//	    check = false;
//	    yield( rand() % 3 );
//	    f[2].reset();
	} // _Select
#if 0
	{
	    UPP::UnarySelector < __typeof__ ( f [ 0 ] ) , int > uSelector0 ( ( f [ 0 ] ) , 3 ) ;
	    UPP::UnarySelector < __typeof__ ( f [ 1 ] ) , int > uSelector1 ( ( f [ 1 ] ) , 4 ) ;
	    UPP::BinarySelector < UPP::OrCondition, __typeof__ ( uSelector0 ), __typeof__ ( uSelector1 ), int > uSelector2 ( uSelector0, uSelector1 ) ;
	    UPP::Executor < __typeof__ ( uSelector2 ) > uExecutor_ ( uSelector2 ) ;
	  _U_S0001_0000 :
	    switch ( uExecutor_ . nextAction ( ) ) {
	      case 0 : goto _U_S0001_0000e ;
	      case 3 : goto _U_S0001_0003 ;
	      case 4 : goto _U_S0001_0004 ;
	    }
	  _U_S0001_0003 : {
		{
		    std :: osacquire ( std :: cerr ) << "f0" << endl ;
		    check = false ;
		    yield ( rand ( ) % 3 ) ;
		    f [ 0 ] . reset ( ) ;
		    std :: osacquire ( std :: cerr ) << "f0 available " << f [ 0 ] . available ( ) << endl ;
		}
	    }
	    goto _U_S0001_0000 ;
	  _U_S0001_0004 : {
		{
		    std :: osacquire ( std :: cerr ) << "f1" << endl ;
		    check = false ;
		    yield ( rand ( ) % 3 ) ;
		    f [ 1 ] . reset ( ) ;
		    std :: osacquire ( std :: cerr ) << "f1 available " << f [ 1 ] . available ( ) << endl ;
		}
	    }
	    goto _U_S0001_0000 ;
	  _U_S0001_0000e : ;
	}
#endif
	std :: osacquire ( std :: cerr ) << "loop" << endl ;
    } // for
#endif
    for ( unsigned int i = 0; i < N; i += 1 ) {
	delete workers[i];
    } // for
#if 0
    {
	Future_ISM<int> f1, f2, f3, f4, f5, f6, f7, f8;
	(
	    _Select( f1 ) {}
	    and _Select( f2 ) {}
	  or
	    _Select( f2 ) {}
	    and _Select( f3 ) {}
	) and (
	    _Select( f1 ) {}
	    and _Select( f2 ) {}
	  or
	    _Select( f2 ) {}
	    and _Select( f3 ) {}
	)
    }
#endif
    cout << "successful completion" << endl;
} // uMain::main

// Local Variables: //
// compile-command: "u++ Futures.cc" //
// End: //
