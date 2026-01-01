//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1997
// 
// EHM1.cc -- testing exception handling mechanism
// 
// Author           : Peter A. Buhr
// Created On       : Wed Nov 26 23:06:25 1997
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Dec 31 08:22:57 2025
// Update Count     : 295
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


#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;


//######################### uBaseCoroutine::uFailure #########################


_Exception XXX {
  public:
	XXX( const char * const msg ) : uBaseException( msg ) {}
};

_Exception YYY {
  public:
	YYY( const char * msg ) { setMsg( msg ); }
};

_Coroutine CoroutineDummy1 {
	bool res;
	void main() {
		if ( res ) {
			_Resume YYY( "resume test" );
		} else {
			_Throw XXX( "throw test" );
		} // if
	} // CoroutineDummy1::main
  public:
	CoroutineDummy1( bool r ) : res( r ) {}
	void mem() { resume(); }
}; // CoroutineDummy1

_Coroutine CoroutineDummy2 {
	void main() {
		CoroutineDummy1 dummy1( false );
		dummy1.mem();
	} // CoroutineDummy1::main
  public:
	void mem() { resume(); }
}; // CoroutineDummy1


//######################### uBaseCoroutine::UnhandledException #########################


_Task TaskDummy0 {
	void main() {
		_Throw XXX( "throw test" );
	} // TaskDummy0::main
}; // TaskDummy0

_Task TaskDummy1 {
	void main() {
		TaskDummy0 t0;
	} // TaskDummy1::main
}; // TaskDummy1


//######################### uCondition::uWaitingFailure #########################


_Task TaskDummy2 {
	uCondition x;
  public:
	void mem() {
		x.wait();
	} // TaskDummy2::mem
  private:
	void main() {
		_Accept( mem );
	} // TaskDummy2::main
}; // TaskDummy2

_Task TaskDummy3 {
	TaskDummy2 & t1;

	void main() {
#if 0
		t1.mem();
#endif
		try {
			t1.mem();
		} catch( uCondition::WaitingFailure & ex ) {
			osacquire( cout ) << "handled exception uCondition::WaitingFailure : task " << ex.source().getName() << " (" << &(ex.source()) << ") found blocked task" << " " << uThisTask().getName() << " (" << &uThisTask() << ") on condition variable " << &ex.conditionId() << endl << endl;
		} // try
	} // TaskDummy3::main
  public:
	TaskDummy3( TaskDummy2 & t1 ) : t1( t1 ) {}
}; // TaskDummy3


//######################### uMutexFailure::uRendezvousFailure #########################


_Task TaskDummy4 {
	uBaseTask & t;
  public:
	TaskDummy4( uBaseTask & t ) : t( t ) {}
	void mem() {
	} // TaskDummy4::mem
  private:
	void main() {
		_Resume XXX( "test" ) _At t;
#if 0
		_Accept( mem );
#endif
		try {
			_Accept( mem );
		} catch ( uMutexFailure::RendezvousFailure & ex ) {
			osacquire( cout ) << "handled exception uMutexFailure::RendezvousFailure : accepted call fails from task " << ex.sourceName() << " (" << &(ex.source()) << ") to mutex member of task " << uThisTask().getName() << " (" << &uThisTask() << ")" << endl << endl;
		} // try
	} // TaskDummy4::main
}; // TaskDummy4


//######################### uMutexFailure::uEntryFailure (acceptor/signalled stack) #########################


_Task TaskDummy5 {
	uCondition temp;
  public:
	void mem() {
		temp.wait();
	} // TaskDummy5::mem
  private:
	void main() {
		_Accept( mem );									// let TaskDummy6 in so it can wait
		temp.signal();									// put TaskDummy6 on A/S stack
		_Accept( ~TaskDummy5 );							// program main is calling the destructor
	} // TaskDummy5::main
}; // TaskDummy5

_Task TaskDummy6 {
	TaskDummy5 & t4;
  public:
	TaskDummy6( TaskDummy5 & t4 ) : t4( t4 ) {}
  private:
	void main() {
#if 0
		t4.mem();
#endif
		try {
			t4.mem();
		} catch( uMutexFailure::EntryFailure & ex ) {
			osacquire( cout ) << "handled exception uMutexFailure::EntryFailure : while executing mutex destructor, task " << ex.source().getName() << " (" << &(ex.source()) << ") found task " << uThisTask().getName() << " (" << &uThisTask() << ") " << ex.message() << endl << endl;
		} catch( ... ) {
			abort( "invalid exception" );
		} // try
	} // TaskDummy6::main
}; // TaskDummy6


//######################### uMutexFailure::uEntryFailure (entry queue) #########################


_Task TaskDummy7 {
  public:
	void mem() {
	} // TaskDummy7::mem
  private:
	void main() {
		_Accept( ~TaskDummy7 );
	} // TaskDummy7::main
};

_Task TaskDummy8 {
	TaskDummy7 & t6;
  public:
	TaskDummy8( TaskDummy7 & t6 ) : t6( t6 ) {}
  private:
	void main() {
#if 0
		t6.mem();
#endif
		try {
			t6.mem();
		} catch( uMutexFailure::EntryFailure & ex ) {
			osacquire( cout ) << "handled exception uMutexFailure::EntryFailure : while executing mutex destructor, task " << ex.source().getName() << " (" << &(ex.source()) << ") found task " << uThisTask().getName() << " (" << &uThisTask() << ") " << ex.message() << endl << endl;
		} catch( ... ) {
			abort( "invalid exception" );
		} // try
	} // TaskDummy8::main
}; // TaskDummy8


int main() {
	//######################### uBaseCoroutine::UnhandledException #########################
	{
		CoroutineDummy1 dummy1( true );

		try {
			dummy1.mem();
		} catch( uBaseCoroutine::UnhandledException & ex ) {
			osacquire( cout ) << "handled exception uBaseCoroutine::UnhandledException : in coroutine " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") raised non-locally from resumed coroutine " << ex.sourceName() << " (" << &(ex.source()) << ") because of " << ex.message() << "." << endl;
			try {
				osacquire( cout ) << "before trigger" << endl;
				ex.triggerCause();
				osacquire( cout ) << "after trigger" << endl << endl;
			} _CatchResume( YYY & ) {
				osacquire( cout ) << "CatchResume YYY" << endl;
			} // try
		} catch( ... ) {
			abort( "invalid exception" );
		} // try
	}
	{
		CoroutineDummy2 dummy2;

		try {
			dummy2.mem();
		} catch( uBaseCoroutine::UnhandledException & ex ) {
			osacquire( cout ) << "handled exception uBaseCoroutine::UnhandledException : in coroutine " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") raised non-locally from resumed coroutine " << ex.sourceName() << " (" << &(ex.source()) << ") because of " << ex.message() << " through " << ex.unhandled() << " unhandled exceptions." << endl;

			osacquire( cout ) << "Now triggering exception" << endl;
			try {
				ex.triggerCause();
			} _CatchResume ( uBaseCoroutine::UnhandledException & e ) {
				osacquire( cout ) << "Now triggering embedded UnhandledException" << endl;    
				e.triggerCause();
				abort();
			} catch ( XXX & ex ) {
				osacquire( cout ) << "caught XXX : " << ex.message() << endl << endl;
			} catch ( ... ) {
				abort( "invalid exception" );
			} // try
		} catch( ... ) {
			abort( "invalid exception" );
		} // try
	}

	//######################### uBaseCoroutine::UnhandledException #########################
	{
		try {
			TaskDummy1 t1;
		} _CatchResume ( uBaseTask::UnhandledException & ex ) {
			osacquire( cout ) << "handled exception uBaseTask::UnhandledException : in task " << uThisTask().getName() << " (" << &uThisTask() << ") raised non-locally from joined task " << ex.sourceName() << " (" << &(ex.source()) << ") because of " << ex.message() << "." << endl;
			try {
				osacquire( cout ) << "Now triggering embedded UnhandledException" << endl;
				ex.triggerCause();
				abort();
			} catch ( XXX & ex ) {
				osacquire( cout ) << "caught XXX : " << ex.message() << endl << endl;
			} catch ( ... ) {
				abort( "invalid exception" );
			} // try
		} // try
	}

	//######################### uCondition::uWaitingFailure #########################

	TaskDummy2 * t1 = new TaskDummy2;
	TaskDummy3 * t2 = new TaskDummy3( *t1 );
	delete t1;						// delete t1 with t2 blocked on condition variable
	delete t2;

	//######################### uMutexFailure::uRendezvousFailure #########################
	{
		TaskDummy4 t3( uThisTask() );
		try {
			_Enable {
				t3.mem();
			} // _Enable
		} catch( XXX & ex ) {
		} catch( ... ) {
			abort( "invalid exception" );
		} // try
	}

	//######################### uMutexFailure::uEntryFailure (acceptor/signalled stack) #########################

	TaskDummy5 * t4 = new TaskDummy5;
	TaskDummy6 * t5 = new TaskDummy6( *t4 );
	delete t4;
	delete t5;

	//######################### uMutexFailure::uEntryFailure (entry queue) #########################

	TaskDummy7 * t6 = new TaskDummy7;
	TaskDummy8 * t7 = new TaskDummy8( *t6 );
	delete t6;
	delete t7;

	cout << "successful completion" << endl;
} // main
