//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// Bench.cc -- Timing benchmarks for the basic features in uC++.
// 
// Author           : Peter A. Buhr
// Created On       : Thu Feb 15 22:03:16 1990
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Sep  7 22:35:40 2016
// Update Count     : 443
// 

#include <iostream>
using std::cerr;
using std::osacquire;
using std::endl;

unsigned int uDefaultPreemption() {
    return 0;
} // uDefaultPreemption

#include "Time.h"

//=======================================
// time class
//=======================================

class ClassDummy {
    static volatile int i;				// prevent dead-code removal
  public:
    ClassDummy() __attribute__(( noinline )) { i = 1; }
    int bidirectional( volatile int a, int, int, int ) __attribute__(( noinline )) {
	return a;
    } // ClassDummy::bidirectional
}; // ClassDummy
volatile int ClassDummy::i;

void BlockClassCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	ClassDummy dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // BlockClassCreateDelete

void DynamicClassCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	ClassDummy *dummy = new ClassDummy;
	delete dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // DynamicClassCreateDelete

void ClassBidirectional( int N ) {
    long long int StartTime, EndTime;
    ClassDummy dummy;
    volatile int rv __attribute__(( unused ));

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	rv = dummy.bidirectional( 1, 2, 3, 4 ); 
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // ClassBidirectional

//=======================================
// time coroutine
//=======================================

_Coroutine CoroutineDummy {
    void main() {
    } // CoroutineDummy::main
  public:
    int bidirectional( volatile int a, int, int, int ) __attribute__(( noinline )) {
	return a;
    } // CoroutineDummy::bidirectional
}; // CoroutineDummy

void BlockCoroutineCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	CoroutineDummy dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // BlockCoroutineCreateDelete

void DynamicCoroutineCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	CoroutineDummy *dummy = new CoroutineDummy;
	delete dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // DynamicCoroutineCreateDelete

void CoroutineBidirectional( int N ) {
    long long int StartTime, EndTime;
    CoroutineDummy dummy;
    volatile int rv __attribute__(( unused ));

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	rv = dummy.bidirectional( 1, 2, 3, 4 ); 
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // CoroutineBidirectional

_Coroutine CoroutineResume {
    int N;

    void main() {
	for ( int i = 1; i <= N; i += 1 ) {
	    suspend();
	} // for
    } // CoroutineResume::main
  public:
    CoroutineResume( int N ) {
	CoroutineResume::N = N;
    } // CoroutineResume::CoroutineResume

    void resumer() {
	long long int StartTime, EndTime;

	StartTime = Time();
	for ( int i = 1; i <= N; i += 1 ) {
	    resume();
	} // for
	EndTime = Time();
	osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
    } // CoroutineResume::resumer
}; // CoroutineResume

//=======================================
// time monitor
//=======================================

_Mutex class MonitorDummy {
  public:
    int bidirectional( volatile int a, int, int, int ) __attribute__(( noinline )) {
	return a;
    } // MonitorDummy::bidirectional
}; // MonitorDummy

void BlockMonitorCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	MonitorDummy dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // BlockMonitorCreateDelete

void DynamicMonitorCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	MonitorDummy *dummy = new MonitorDummy;
	delete dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // DynamicMonitorCreateDelete

void MonitorBidirectional( int N ) {
    long long int StartTime, EndTime;
    MonitorDummy dummy;
    volatile int rv __attribute__(( unused ));

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	rv = dummy.bidirectional( 1, 2, 3, 4 ); 
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // MonitorBidirectional

_Monitor Monitor {
    uCondition condA, condB;
  public:
    volatile int here;

    Monitor() {
	here = 0;
    }; // Monitor::Monitor

    int caller( int a ) {
	return a;
    } // Monitor::caller

    void acceptor( int N ) {
	long long int StartTime, EndTime;

	here = 1;					// indicate that the acceptor is in place

	StartTime = Time();
	for ( int i = 1; i <= N; i += 1 ) {
	    _Accept( caller );
	} // for
	EndTime = Time();
	osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
    } // Monitor::acceptor

    void sigwaiterA( int N ) {
	long long int StartTime, EndTime;

	StartTime = Time();
	for ( int i = 1;; i += 1 ) {
	    condA.signal();
	  if ( i > N ) break;
	    condB.wait();
	} // for
	EndTime = Time();
	osacquire( cerr ) << "\t " << ( ( EndTime - StartTime ) / N ) / 2;
    } // Monitor::sigwaiterA

    void sigwaiterB( int N ) {
	for ( int i = 1;; i += 1 ) {
	    condB.signal();
	  if ( i > N ) break;
	    condA.wait();
	} // for
    } // Monitor::sigwaiterB
}; // Monitor

_Task MonitorAcceptorPartner {
    int N;
    Monitor &m;

    void main() {
	m.acceptor( N );
    } // MonitorAcceptorPartner::main
  public:
    MonitorAcceptorPartner( int N, Monitor &m ) : m( m ) {
	MonitorAcceptorPartner::N = N;
    } // MonitorAcceptorPartner
}; // MonitorAcceptorPartner

_Task MonitorAcceptor {
    int N;

    void main() {
	Monitor m;
	MonitorAcceptorPartner partner( N, m );
	volatile int rv __attribute__(( unused ));

	while ( m.here == 0 ) yield();			// wait until acceptor is in monitor

	for ( int i = 1; i <= N; i += 1 ) {
	    rv = m.caller( 1 );
	} // for
    } // MonitorAcceptor::main
  public:
    MonitorAcceptor( int NoOfTimes ) {
	N = NoOfTimes;
    } // MonitorAcceptor
}; // MonitorAcceptor

_Task MonitorSignallerPartner {
    int N;
    Monitor &m;

    void main() {
	m.sigwaiterB( N );
    } // MonitorSignallerPartner::main
  public:
    MonitorSignallerPartner( int N, Monitor &m ) : m( m ) {
	MonitorSignallerPartner::N = N;
    } // MonitorSignallerPartner
}; // MonitorSignallerPartner

_Task MonitorSignaller {
    int N;

    void main() {
	Monitor m;
	MonitorSignallerPartner partner( N, m );

	m.sigwaiterA( N );
    } // MonitorSignaller::main
  public:
    MonitorSignaller( int NoOfTimes ) {
	N = NoOfTimes;
    } // MonitorSignaller
}; // MonitorSignaller

//=======================================
// time coroutine-monitor
//=======================================

_Mutex _Coroutine CoroutineMonitorDummy {
    void main() {}
  public:
    int bidirectional( volatile int a, int, int, int ) __attribute__(( noinline )) {
	return a;
    } // CoroutineMonitorDummy::bidirectional
}; // CoroutineMonitorDummy

void BlockCoroutineMonitorCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	CoroutineMonitorDummy dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // BlockCoroutineMonitorCreateDelete

void DynamicCoroutineMonitorCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	CoroutineMonitorDummy *dummy = new CoroutineMonitorDummy;
	delete dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // DynamicCoroutineMonitorCreateDelete

void CoroutineMonitorBidirectional( int N ) {
    long long int StartTime, EndTime;
    CoroutineMonitorDummy dummy;
    volatile int rv __attribute__(( unused ));

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	rv = dummy.bidirectional( 1, 2, 3, 4 );
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // CoroutineMonitorBidirectional

_Mutex _Coroutine CoroutineMonitorA {
    int N;
  public:
    volatile int here;

    CoroutineMonitorA() {
	here = 0;
    } // CoroutineMonitorA::CoroutineMonitorA

    int caller( int a ) {
	return a;
    } // CoroutineMonitorA::caller

    void acceptor( int N ) {
	CoroutineMonitorA::N = N;
	resume();
    } // CoroutineMonitorA::acceptor
  private:
    void main() {
	long long int StartTime, EndTime;

	here = 1;

	StartTime = Time();
	for ( int i = 1; i <= N; i += 1 ) {
	    _Accept( caller );
	} // for
	EndTime = Time();
	osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
    }; // CoroutineMonitorA::main
}; // CoroutineMonitorA

_Task CoroutineMonitorAcceptorPartner {
    int N;
    CoroutineMonitorA &cm;

    void main() {
	cm.acceptor( N );
    } // CoroutineMonitorAcceptorPartner::main
  public:
    CoroutineMonitorAcceptorPartner( int N, CoroutineMonitorA &cm ) : cm( cm ) {
	CoroutineMonitorAcceptorPartner::N = N;
    } // CoroutineMonitorAcceptorPartner
}; // CoroutineMonitorAcceptorPartner

_Task CoroutineMonitorAcceptor {
    int N;
    void main();
  public:
    CoroutineMonitorAcceptor( int NoOfTimes ) {
	N = NoOfTimes;
    } // CoroutineMonitorAcceptor
}; // CoroutineMonitorAcceptor

void CoroutineMonitorAcceptor::main() {
    CoroutineMonitorA cm;
    CoroutineMonitorAcceptorPartner partner( N, cm );
    volatile int rv __attribute__(( unused ));

    while ( cm.here == 0 ) yield();
    
    for ( int i = 1; i <= N; i += 1 ) {
	rv = cm.caller( 1 );
    } // for
} // CoroutineMonitorAcceptor::main

_Mutex _Coroutine CoroutineMonitorResume {
    int N;

    void main() {
	for ( int i = 1; i <= N; i += 1 ) {
	    suspend();
	} // for
    }; // CoroutineMonitorResume::main
  public:
    CoroutineMonitorResume( int N ) {
	CoroutineMonitorResume::N = N;
    } // CoroutineMonitorResume::CoroutineMonitorResume

    void resumer() {
	long long int StartTime, EndTime;

	StartTime = Time();
	for ( int i = 1; i <= N; i += 1 ) {
	    resume();
	} // for
	EndTime = Time();
	osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
    }; // CoroutineMonitorResume::resumer
}; // CoroutineMonitorResume

_Mutex _Coroutine CoroutineMonitorB {
    int N;
    uCondition condA, condB;

    void main() {
	long long int StartTime, EndTime;

	StartTime = Time();
	for ( int i = 1;; i += 1 ) {
	    condA.signal();
	  if ( i > N ) break;
	    condB.wait();
	} // for
	EndTime = Time();
	osacquire( cerr ) << "\t " << ( ( EndTime - StartTime ) / N ) / 2;
    }; // CoroutineMonitorB::main
  public:
    void sigwaiterA( int N ) {
	CoroutineMonitorB::N = N;
	resume();
    } // CoroutineMonitorB::sigwaiterA

    void sigwaiterB( int N ) {
	for ( int i = 1;; i += 1 ) {
	    condB.signal();
	  if ( i > N ) break;
	    condA.wait();
	} // for
    } // CoroutineMonitorB::sigwaiterB
}; // CoroutineMonitorB

_Task CoroutineMonitorSignallerPartner {
    int N;
    CoroutineMonitorB &cm;

    void main() {
	cm.sigwaiterB( N );
    } // CoroutineMonitorSignallerPartner::main
  public:
    CoroutineMonitorSignallerPartner( int N, CoroutineMonitorB &cm ) : cm( cm ) {
	CoroutineMonitorSignallerPartner::N = N;
    } // CoroutineMonitorSignallerPartner
}; // CoroutineMonitorSignallerPartner

_Task CoroutineMonitorSignaller {
    int N;

    void main() {
	CoroutineMonitorB cm;
	CoroutineMonitorSignallerPartner partner( N, cm );

	cm.sigwaiterA( N );
    } // CoroutineMonitorSignaller::main
  public:
    CoroutineMonitorSignaller( int NoOfTimes ) {
	N = NoOfTimes;
    } // CoroutineMonitorSignaller
}; // CoroutineMonitorSignaller


//=======================================
// time task
//=======================================

_Task TaskDummy {
    void main() {
    } // TaskDummy::main
}; // TaskDummy

void BlockTaskCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	TaskDummy dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // BlockTaskCreateDelete

void DynamicTaskCreateDelete( int N ) {
    long long int StartTime, EndTime;

    StartTime = Time();
    for ( int i = 0; i < N; i += 1 ) {
	TaskDummy *dummy = new TaskDummy;
	delete dummy;
    } // for
    EndTime = Time();
    osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
} // DynamicTaskCreateDelete

_Task TaskAcceptorPartner {
    int N;
  public:
    int caller( int a ) {
	return a;
    } // TaskAcceptorPartner::caller

    TaskAcceptorPartner( int N ) {
	TaskAcceptorPartner::N = N;
    } // TaskAcceptorPartner
  private:
    void main() {
	long long int StartTime, EndTime;
    
	StartTime = Time();
	for ( int i = 1; i <= N; i += 1 ) {
	    _Accept( caller );
	} // for
	EndTime = Time();
	osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
    } // TaskAcceptorPartner::main
}; // TaskAcceptorPartner

_Task TaskAcceptor {
    int N;

    void main() {
	TaskAcceptorPartner partner( N );
	volatile int rv __attribute__(( unused ));

	for ( int i = 1; i <= N; i += 1 ) {
	    rv = partner.caller( 1 );
	} // for
    } // TaskAcceptor::main
  public:
    TaskAcceptor( int NoOfTimes ) {
	N = NoOfTimes;
    } // TaskAcceptor
}; // TaskAcceptor

_Task TaskSignallerPartner {
    int N;
    uCondition condA, condB;
  public:
    void sigwaiter( int N ) {
	for ( int i = 1;; i += 1 ) {
	    condB.signal();
	  if ( i > N ) break;
	    condA.wait();
	} // for
    } // TaskSignallerPartner::sigwaiter

    TaskSignallerPartner( int N ) {
	TaskSignallerPartner::N = N;
    } // TaskSignallerPartner
  private:
    void main() {
	long long int StartTime, EndTime;

	_Accept( sigwaiter );

	StartTime = Time();
	for ( int i = 1;; i += 1 ) {
	    condA.signal();
	  if ( i > N ) break;
	    condB.wait();
	} // for
	EndTime = Time();
	osacquire( cerr ) << "\t " << ( ( EndTime - StartTime ) / N ) / 2;
    } // TaskSignallerPartner::main
}; // TaskSignallerPartner

_Task TaskSignaller {
    int N;

    void main() {
	TaskSignallerPartner partner( N );

	partner.sigwaiter( N );
    } // TaskSignaller::main
  public:
    TaskSignaller( int NoOfTimes ) {
	N = NoOfTimes;
    } // TaskSignaller
}; // TaskSignaller

//=======================================
// time context switch
//=======================================

_Task ContextSwitch {
    int N;

    void main() {    
	long long int StartTime, EndTime;

	StartTime = Time();
	for ( int i = 1; i <= N; i += 1 ) {
	    uYieldNoPoll();
	} // for
	EndTime = Time();
	osacquire( cerr ) << "\t " << ( EndTime - StartTime ) / N;
    } // ContextSwitch::main
  public:
    ContextSwitch( int N ) {
	ContextSwitch::N = N;
    } // ContextSwitch
}; // ContextSwitch

//=======================================
// benchmark driver
//=======================================

void uMain::main() {
#if defined( __U_AFFINITY__ )
    // Prevent moving onto cold CPUs during benchmark.
    cpu_set_t mask;
    uThisProcessor().getAffinity( mask );		// get allowable CPU set
    unsigned int cpu;
    for ( cpu = 0;; cpu += 1 ) {			// look for first available CPU
      if ( cpu == CPU_SETSIZE ) uAbort( "could not find available CPU to set affinity" );
      if ( CPU_ISSET( cpu, &mask ) ) break;
    } // for
    CPU_ZERO( &mask );
    CPU_SET( cpu, &mask );				// create mask with specific CPU
    uThisProcessor().setAffinity( mask );		// execute benchmark on this CPU
#endif // __U_AFFINITY__

    const int NoOfTimes =
#if defined( __U_DEBUG__ )				// takes longer so run fewer iterations
	100000;
#else
	1000000;
#endif // __U_DEBUG__

    osacquire( cerr ) << "\t\tcreate\tcreate\t16i/4o\t16i/4o\tresume/\tsignal/" << endl;
    osacquire( cerr ) << "(nsecs)";
    osacquire( cerr ) << "\t\tdelete/\tdelete/\tbytes\tbytes\tsuspend\twait" << endl;
    osacquire( cerr ) << "\t\tblock\tdynamic\tdirect\taccept\tcycle\tcycle" << endl;

    osacquire( cerr ) << "class\t";
    BlockClassCreateDelete( NoOfTimes );
    DynamicClassCreateDelete( NoOfTimes );
    ClassBidirectional( NoOfTimes );
    osacquire( cerr ) << "\t N/A\t N/A\t N/A";
    osacquire( cerr ) << "\t" << endl;

    osacquire( cerr ) << "coroutine";
    BlockCoroutineCreateDelete( NoOfTimes );
    DynamicCoroutineCreateDelete( NoOfTimes );
    CoroutineBidirectional( NoOfTimes );
    osacquire( cerr ) << "\t N/A";
    {
	CoroutineResume resumer( NoOfTimes );
	resumer.resumer();
    }
    osacquire( cerr ) << "\t N/A";
    osacquire( cerr ) << "\t" << endl;

    osacquire( cerr ) << "monitor\t";
    BlockMonitorCreateDelete( NoOfTimes );
    DynamicMonitorCreateDelete( NoOfTimes );
    MonitorBidirectional( NoOfTimes );
    {
	MonitorAcceptor m( NoOfTimes );
    }
    osacquire( cerr ) << "\t N/A";
    {
	MonitorSignaller m( NoOfTimes );
    }
    osacquire( cerr ) << "\t" << endl;

    osacquire( cerr ) << "cormonitor";
    BlockCoroutineMonitorCreateDelete( NoOfTimes );
    DynamicCoroutineMonitorCreateDelete( NoOfTimes );
    CoroutineMonitorBidirectional( NoOfTimes );
    {
	CoroutineMonitorAcceptor cm( NoOfTimes );
    }
    {
	CoroutineMonitorResume resumer( NoOfTimes );
	resumer.resumer();
    }
    {
	CoroutineMonitorSignaller cm( NoOfTimes );
    }
    osacquire( cerr ) << "\t" << endl;

    osacquire( cerr ) << "task\t";
    BlockTaskCreateDelete( NoOfTimes );
    DynamicTaskCreateDelete( NoOfTimes );
    osacquire( cerr ) << "\t N/A";
    {
	TaskAcceptor t( NoOfTimes );
    }
    osacquire( cerr ) << "\t N/A";
    {
	TaskSignaller t( NoOfTimes );
    }
    osacquire( cerr ) << "\t" << endl;

    osacquire( cerr ) << "cxt sw\t";
    {
	ContextSwitch dummy( NoOfTimes );		// context switch
    }
    osacquire( cerr ) << "\t" << endl;
} // uMain::main

// Local Variables: //
// compile-command: "../../bin/u++ -O2 -nodebug Bench.cc" //
// End: //
