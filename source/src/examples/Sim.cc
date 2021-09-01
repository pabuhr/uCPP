//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// Sim.cc -- This program performs a benchmark test on the concurrency facilities of the multiprocessor uC++.
// 
// Author           : Peter A. Buhr
// Created On       : Fri Aug 16 13:51:34 1991
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu May 20 21:39:19 2021
// Update Count     : 144
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


#include "uCalibrate.h"
#include <iostream>
using std::cout;
using std::endl;

unsigned int uDefaultPreemption() {
	return 1;
} // uDefaultPreemption

//unsigned int uDefaultSpin() {
//	return 0;
//} // uDefaultSpin

_Task Worker {
	unsigned int NoWorkers, PerWorker, Extra;

	void main();
  public:
	Worker( unsigned int NoWorkers, unsigned int PerWorker, unsigned int Extra ) : NoWorkers( NoWorkers ), PerWorker( PerWorker ), Extra( Extra ) {
	} // Worker::Worker
}; // Worker

void Worker::main() {
	unsigned int leftworkers = 0, rightworkers, work, extrawork;
	Worker *LeftWorker = nullptr, *RightWorker = nullptr;

	work = PerWorker;
	if ( Extra > 0 ) {										// if there are still extras, take one
		Extra -= 1;
		work += 1;
	} // if
	NoWorkers -= 1;										// take a worker
	if ( NoWorkers > 0 ) {								// create siblings
		leftworkers = NoWorkers / 2;
		extrawork = Extra / 2;
		if ( leftworkers != 0 ) {
			LeftWorker = new Worker( leftworkers, PerWorker, extrawork );
		} // if
		rightworkers = NoWorkers - leftworkers;
		extrawork = Extra - extrawork;
		RightWorker = new Worker( rightworkers, PerWorker, extrawork );
	} // if

	for ( unsigned int i = 1; i <= work; i += 1 ) {
		for ( volatile unsigned long long int j = 1; j <= ITERATIONS_FOR_100USECS; j += 1 ) { // 0.1 millisecond loop
		} // for
	} // for

	if ( NoWorkers > 0 ) {								// wait for sibling, if any
		if ( leftworkers != 0 ) {
			delete LeftWorker;
		} // if
		delete RightWorker;
	} // if
} // Worker::main

int main( int argc, char *argv[] ) {
	unsigned int NoWorkers, NoProcessors, work;

	if ( argc != 4 ) {
		abort( "Usage: no.-processors  no.-worker-tasks  amount-of-task-work" );
	} // if

	NoProcessors = atoi( argv[1] );
	NoWorkers = atoi( argv[2] );
	work = atoi( argv[3] );

	uProcessor **processor = new uProcessor *[NoProcessors - 1];
	for ( unsigned int i = 0; i < NoProcessors - 1; i += 1 ) {
		processor[ i ] = new uProcessor;
	} // for
	{
		Worker worker( NoWorkers, work / NoWorkers, work % NoWorkers );
	}
	for ( unsigned int i = 0; i < NoProcessors - 1; i += 1 ) {
		delete processor[ i ];
	} // for
	delete [] processor;

	cout << "successful completion" << endl;
} // uMain

// Local Variables: //
// compile-command: "../../bin/u++ -multi -g Sim.cc" //
// End: //
