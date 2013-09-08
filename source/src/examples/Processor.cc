#include <iostream>
using std::cout;
using std::endl;

volatile bool stop = false;

_Task Worker {
    void main() {
	while ( ! stop ) {
	    yield();
	}
    }
};

void uMain::main() {
    uProcessor *processor2 = NULL;
    {
	Worker tasks[2];

	uProcessor *processor1 = new uProcessor();

	while ( &uThisProcessor() != processor1 ) {
	    yield();
	} // while

	processor2 = new uProcessor();

//	delete processor2;
	delete processor1;

	stop = true;
    }
    cout << "here" << endl;
    delete processor2;
} // uMain::main

// Local Variables: //
// compile-command: "../../bin/u++ -g -multi Processor.cc" //
// End: //
