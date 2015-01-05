//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Philipp E. Lim and Ashif S. Harji 1996
// 
// PeriodicTaskTest.cc -- 
// 
// Author           : Philipp E. Lim and Ashif S. Harji
// Created On       : Tue Jul 23 14:48:37 1996
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 18 11:08:32 2010
// Update Count     : 129
// 

#include <uRealTime.h>
#include <uDeadlineMonotonic.h>
#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;
#include <iomanip>
using std::setw;

const int Delay = 100000;
uTime Start;						// global start time for all tasks


class AcrossCxtSw : public uContext {
    uTime &clock, beginCxtSw;
  public:
    AcrossCxtSw( uTime &clock ) : clock( clock ) {
    } // AcrossCxtSw::AcrossCxtSw

    void save(){
	beginCxtSw = uThisProcessor().getClock().getTime();
    } // AcrossCxtSw::save

    void restore(){
	clock += uThisProcessor().getClock().getTime() - beginCxtSw;
    } // AcrossCxtSw::restore
}; // AcrossCxtSw


// This real-time task is periodic with a specific duration and computation time (C). However, due to pre-emption from
// tasks with higher priority and time slicing, a direct calculation to simulate computation time does not work because
// time spent executing other tasks is not excluded from the calculation of C.  In order to compensate for this, a
// function is called during each context switch to calculate the amount of time spend outside the task.  By adding this
// time to the task's calculated stop time after each context switch, an accurate simulation of the computation time, C,
// is possible.

_PeriodicTask TestTask {
    uDuration C;
    int id;

    void main() {
	uTime starttime, delay, currtime, endtime;

	starttime = uThisProcessor().getClock().getTime();
	osacquire( cout ) << setw(3) << starttime - ::Start << "\t" << id << " Beginning." << endl;

	// The loop below must deal with asynchronous advancing of variable "delay" during context switches. A problem
	// occurs when "delay" is loaded into a register for the comparison, a context switch occurs, and "delay" is
	// advanced, but the old value of "delay" is used in a comparison with the current time. This situation causes a
	// premature exit from the loop because the old delay value is less than the current time.  To solve the
	// problem, the current time value is saved *before* the comparison.  Thus, whether the delay value is the old
	// or new (i.e., becomes larger) value, the comparision does not cause a premature exit.

	delay = uThisProcessor().getClock().getTime() + C;
	{
	    AcrossCxtSw acrossCxtSw( delay );		// cause delay to advance across a context switch interval
	    do {
		for ( int i = 0; i < Delay; i += 1 );	// don't spend too much time in non-interruptible clock routine
		currtime = uThisProcessor().getClock().getTime();
	    } while ( delay > currtime );
	}
	endtime = uThisProcessor().getClock().getTime();
	osacquire( cout ) << setw(3) << endtime - ::Start << "\t" << id << " Ending " <<
	    setw(3) << endtime - starttime << " seconds later" << endl;
    } // TestTask::main
  public:
    TestTask( int id, uDuration period, uDuration deadline, uCluster &clust ) :
	    uPeriodicBaseTask( period, uTime(0), uThisProcessor().getClock().getTime()+90, deadline, clust ),
	    C( deadline ), id( id ) {
    } // TestTask::TestTask
}; // TestTask

void uMain::main() {
    uDeadlineMonotonic rq;				// create real-time scheduler
    uRealTimeCluster rtCluster( rq );			// create real-time cluster with scheduler
    uProcessor *processor;
    {
	TestTask t1( 1, 15, 5, rtCluster );
	TestTask t2( 2, 30, 9, rtCluster );
	TestTask t3( 3, 60, 19, rtCluster );

	osacquire( cout ) << "Time  \t\tTaskID" << endl;

	::Start = uThisProcessor().getClock().getTime();
	processor = new uProcessor( rtCluster );	// now create the processor to do the work
    } // wait for t1, t2, t3 to finish
    delete processor;
    cout << "successful completion" << endl;
} // uMain::main
