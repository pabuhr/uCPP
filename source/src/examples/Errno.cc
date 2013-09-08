#include <iostream>
using namespace std;

_Task Worker {
    void main() {
	for ( int i = 0; i < 1000000; i += 1 ) {
	    errno = i;
	    yield();
	    if ( i != errno ) uAbort( "Error: interference on errno %p %p %d %d\n", &uThisTask(), &uThisProcessor(), i, errno );
	} // for
    } // Worker::main
}; // Worker

void uMain::main() {
    uProcessor p[3];
    Worker w[10];
}
