#include <iostream>
using namespace std;
#include <uActor.h>

struct TMsg : public uActor::TraceMsg { int cnt = 0; } tmsg; // traceable message

_CorActor Trace {
	Allocation receive( Message & msg ) {
		Case( TMsg, msg ) { resume(); }
		else Case( StopMsg, msg ) return Finished;
		return Nodelete;
	}
	void main() {
		cout << "Build Trace" << endl;
		for ( int i = 0; i < 4; i += 1 ) {
			tmsg.print();  *this | tmsg;				// send message to self
			suspend();
		}
		cout << "Move Cursor Back" << endl;
		for ( int i = 0; i < 4; i += 1 ) {
			tmsg.print();  tmsg.Return();				// move cursor back 1 hop
			suspend();
		}
		cout << "Move Cursor Head" << endl;
		tmsg.resume(); tmsg.print();					// move cursor to head
		suspend();
		cout << "Delete Trace" << endl;
		tmsg.retSender();  tmsg.reset();  tmsg.print();	// move cursor and delete all hops
		suspend();
		cout << "Build Trace" << endl;
		for ( int i = 0; i < 4; i += 1 ) {
			tmsg.print();  *this | tmsg;				// send message to self
			suspend();
		}
		cout << "Erase trace" << endl;
		tmsg.erase();  tmsg.print();					// remove hops, except cursor
		*this | uActor::stopMsg;
		suspend();
	}
}; // Sender

int main() {
	uActor::start();									// start actor system
	Trace trace;
	trace | tmsg;
	uActor::stop();										// wait for all actors to terminate
} // main

// Local Variables: //
// tab-width: 4 //
// End: //
