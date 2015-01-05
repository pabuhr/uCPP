#include <uC++.h>
#include <uFuture.h>
#include <iostream>
using namespace std;

int routine() {
	// preform work
	return 3;
}
struct Functor {										// closure: allowing arguments to work
	double x;
	double operator()() {
		// preform work
		return x;
	}
	Functor( double x ) : x( x ) {}
} functor( 4.5 );

void uMain::main() {
	uExecutor executor;
	Future_ISM<int> fi[ 10 ];
	Future_ISM<double> fd[10];
	for ( int i = 0; i < 10; i += 1 ) {
		executor.submit( fi[i], routine );				// think: fi[i] = executor.submit( routine )
		executor.submit( fd[i], functor );				// think: fd[i] = executor.submit( functor )
	} // for
	for ( int i = 0; i < 10; i += 1 ) {
		cout << fi[i]() << " " << fd[i]() << " ";		// wait for results
	} // for
	cout << endl;
}

// Local Variables: //
// compile-command: "u++-work test.cc" //
// tab-width: 4 //
// End: //
