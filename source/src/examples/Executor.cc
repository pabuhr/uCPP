#include <iostream>
#include <uFuture.h>
using namespace std;

int routine() {
	return 3;											// preform work
}
struct Functor {										// closure: allows arguments to work
	double val;
	double operator()() {								// function-call operator
		return val;										// preform work
	}
	Functor( double val ) : val( val ) {}
} functor( 4.5 );

void routine2() {
	osacquire( cout ) << -1 << endl;					// preform work
}
struct Functor2 {										// closure: allows arguments to work
	double val;
	void operator()() {									// function-call operator
		osacquire( cout ) << val << endl;				// preform work
	}
	Functor2( double val ) : val( val ) {}
} functor2( 7.3 );

void uMain::main() {
	enum { NoOfRequests = 10 };
	uExecutor executor(3, 1);							// work-pool of threads and processors
	Future_ISM<int> fi[NoOfRequests];
	Future_ISM<double> fd[NoOfRequests];
	Future_ISM<char> fc[NoOfRequests];

	for ( int i = 0; i < NoOfRequests; i += 1 ) {		// send off work for executor
		fi[i] = executor.sendrecv( routine );			//  and get return future
		fd[i] = executor.sendrecv( functor );
		fc[i] = executor.sendrecv( []() { return 'a'; } );
	} // for
	executor.send( routine2 );							// send off work but no return value
	executor.send( functor2 );
	executor.send( []() { osacquire( cout ) << 'd' << endl; } );
	for ( int i = 0; i < NoOfRequests; i += 1 ) {		// wait for results
		osacquire( cout ) << fi[i]() << " " << fd[i]() << " " << fc[i]() << " ";
	} // for
	cout << endl;
}

// Local Variables: //
// tab-width: 4 //
// End: //
