//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// LOOK.cc -- Look Disk Scheduling Algorithm
//
// The LOOK disk scheduling algorithm causes the disk arm to sweep bidirectionally across the disk surface until there
// are no more requests in that particular direction, servicing all requests in its path.
// 
// Author           : Peter A. Buhr
// Created On       : Thu Aug 29 21:46:11 1991
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 18 11:06:04 2010
// Update Count     : 281
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

typedef char Buffer[50];								// dummy data buffer

const int NoOfCylinders = 100;
enum IOStatus { IO_COMPLETE, IO_ERROR };

class IORequest {
  public:
	int track;
	int sector;
	Buffer *bufadr;
	IORequest() {}
	IORequest( int track, int sector, Buffer *bufadr ) {
		IORequest::track = track;
		IORequest::sector = sector;
		IORequest::bufadr = bufadr;
	} // IORequest::IORequest
}; // IORequest

class WaitingRequest : public uSeqable {				// element for a waiting request list
	WaitingRequest( WaitingRequest & );					// no copy
	WaitingRequest &operator=( WaitingRequest & );		// no assignment
  public:
	uCondition block;
	IOStatus status;
	IORequest req;
	WaitingRequest( IORequest req ) {
		WaitingRequest::req = req;
	}
}; // WaitingRequest

class Elevator : public uSequence<WaitingRequest> {
	int Direction;
	WaitingRequest *Current;

	Elevator( Elevator & );								// no copy
	Elevator &operator=( Elevator & );					// no assignment
  public:
	Elevator() {
		Direction = 1;
	} // Elevator::Elevator

	void orderedInsert( WaitingRequest *np ) {
		WaitingRequest *lp;
		for ( lp = head();								// insert in ascending order by track number
			 lp != 0 && lp->req.track < np->req.track;
			 lp = succ( lp ) );
		if ( empty() ) Current = np;					// 1st client, so set Current
		insertBef( np, lp );
	} // Elevator::orderedInsert

	WaitingRequest *remove() {
		WaitingRequest *temp = Current;					// advance to next waiting client
		Current = Direction ? succ( Current ) : pred( Current );
		uSequence<WaitingRequest>::remove( temp );		// remove request

		if ( Current == 0 ) {							// reverse direction ?
			osacquire( cout ) << "Turning" << endl;
			Direction = !Direction;
			Current = Direction ? head() : tail();
		} // if
		return temp;
	} // Elevator::remove
}; // Elevator

_Task DiskScheduler;

_Task Disk {
	DiskScheduler &scheduler;
	void main();
  public:
	Disk( DiskScheduler &scheduler ) : scheduler( scheduler ) {
	} // Disk
}; // Disk

_Task DiskScheduler {
	Elevator PendingClients;							// ordered list of client requests
	uCondition DiskWaiting;								// disk waits here if no work
	WaitingRequest *CurrentRequest;						// request being serviced by disk
	Disk disk;											// start the disk
	IORequest req;
	WaitingRequest diskterm;							// preallocate disk termination request

	void main();
  public:
	DiskScheduler() : disk( *this ), req( -1, 0, 0 ), diskterm( req ) {
	} // DiskScheduler
	IORequest WorkRequest( IOStatus );
	IOStatus DiskRequest( IORequest & );
}; // DiskScheduler

_Task DiskClient {
	DiskScheduler &scheduler;
	void main();
  public:
	DiskClient( DiskScheduler &scheduler ) : scheduler( scheduler ) {
	} // DiskClient
}; // DiskClient

void Disk::main() {
	IOStatus status;
	IORequest work;

	status = IO_COMPLETE;
	for ( ;; ) {
		work = scheduler.WorkRequest( status );
	  if ( work.track == -1 ) break;
		osacquire( cout ) << "Disk main, track:" << work.track << endl;
		yield( 100 );									// pretend to perform an I/O operation
		status = IO_COMPLETE;
	} // for
} // Disk::main

void DiskScheduler::main() {
	uSeqIter<WaitingRequest> iter;						// declared here because of gcc compiler bug

	CurrentRequest = NULL;								// no current request at start
	for ( ;; ) {
		_Accept( ~DiskScheduler ) {						// request from system
			break;
		} or _Accept( WorkRequest ) {					// request from disk
		} or _Accept( DiskRequest ) {					// request from clients
		} // _Accept
	} // for
	
	// two alternatives for terminating scheduling server
#if 0
	for ( ; ! PendingClients.empty(); ) {				// service pending disk requests before terminating
		_Accept( WorkRequest );
	} // for
#else
	WaitingRequest *client;								// cancel pending disk requests before terminating

	for ( iter.over(PendingClients); iter >> client; ) {
		PendingClients.remove();						// remove each client from the list
		client->status = IO_ERROR;						// set failure status
		client->block.signal();							// restart client
	} // for
#endif
	// pending client list is now empty
	
	// stop disk
	PendingClients.orderedInsert( &diskterm );			// insert disk terminate request on list
	
	if ( ! DiskWaiting.empty() ) {						// disk free ?
		DiskWaiting.signal();							// wake up disk to deal with termination request
	} else {
		_Accept( WorkRequest );							// wait for current disk operation to complete
	} // if
} // DiskScheduler::main

IOStatus DiskScheduler::DiskRequest( IORequest &req ) {
	WaitingRequest np( req );							// preallocate waiting list element

	PendingClients.orderedInsert( &np );				// insert in ascending order by track number
	if ( ! DiskWaiting.empty() ) {						// disk free ?
		DiskWaiting.signal();							// reactivate disk
	} // if

	np.block.wait();									// wait until request is serviced

	return np.status;									// return status of disk request
} // DiskScheduler::DiskRequest

IORequest DiskScheduler::WorkRequest( IOStatus status ) {
	if ( CurrentRequest != NULL ) {						// client waiting for request to complete ?
		CurrentRequest->status = status;				// set request status
		CurrentRequest->block.ignal();					// reactivate waiting client
	} // if

	if ( PendingClients.empty() ) {						// any clients waiting ?
		DiskWaiting.wait();								// wait for client to arrive
	} // if

	CurrentRequest = PendingClients.remove();			// remove next client's request
	return CurrentRequest->req;							// return work for disk
} // DiskScheduler::WorkRequest

void DiskClient::main() {
	IOStatus status;
	IORequest req( rand() % NoOfCylinders, 0, 0 );
	
	yield( rand() % 100 );								// don't all start at the same time
	osacquire( cout ) << "enter DiskClient main seeking:" << req.track << endl;
	status = scheduler.DiskRequest( req );
	osacquire( cout ) << "enter DiskClient main seeked to:" << req.track << endl;
} // DiskClient::main

void uMain::main() {
	const int NoOfTests = 20;
	DiskScheduler scheduler;							// start the disk scheduler
	DiskClient *p;

	srand( getpid() );									// initialize random number generator

	p = new DiskClient[NoOfTests]( scheduler );			// start the clients
	delete [] p;										// wait for clients to complete

	cout << "successful completion" << endl;
} // uMain::main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ LOOK.cc" //
// End: //
