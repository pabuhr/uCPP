//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// Philosopher.cc -- Dining Philosophers Problem
// 
//     Philosophers are eating around a dining table; however, the table is set so that there is only one fork between
//     each philosopher. Since two forks are necessary to eat, not all philosophers can eat at the same
//     time. Fortunately, a philosopher is not always eating, she may be just hungry or thinking. Hence, a philosopher
//     can be eating when the philosophers on the left and right are not eating.
// 
// Author           : Peter A. Buhr
// Created On       : Sat Oct 19 14:06:13 1991
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Jul 18 11:09:32 2010
// Update Count     : 205
// 

#include <uSemaphore.h>
#include <iostream>
using std::cout;
using std::cerr;
using std::osacquire;
using std::endl;
#include <iomanip>
using std::setw;

class Table {
    enum State { THINKING, HUNGRY, EATING };

    const int settings;									// number of table settings
    State *state;										// array of Philosopher states
    uSemaphore *self;									// pointer to an array of pointers to semaphores
    uSemaphore lock;									// mutual exclusion lock

    int RightOf( int me ) {
        return ( me + 1 ) % settings;
    } // Table::RightOf

    int LeftOf( int me ) {
        return ( me != 0 ) ? me - 1 : settings - 1;
    } // Table::LeftOf

    void TestBeside( int me ) {
        if ( state[LeftOf( me )] != EATING && state[me] == HUNGRY && state[RightOf( me )] != EATING ) {
            state[me] = EATING;
            self[me].V();
        } // if
    } // Table::TestBeside
  public:
    Table( int settings ) : settings( settings ), lock( 1 ) {
        self = new uSemaphore[settings];
        state = new State[settings];
        for ( int i = 0; i < settings; i += 1 ) {
			self[i].P();								// set all elements to 0
            state[i] = THINKING;
        } // for
    } // Table::Table

    ~Table() {
        delete [] state;
        delete [] self;
    } // Table:~Table

    void pickup( int me ) {
        lock.P();
        state[me] = HUNGRY;
        TestBeside( me );
        lock.V();
        self[me].P();
    } // Table::pickup

    void putdown( int me ) {
        lock.P();
        state[me] = THINKING;
        TestBeside( LeftOf( me ) );
        TestBeside( RightOf( me ) );
        lock.V();
    } // Table::putdown
}; // Table

_Task Philosopher {
    int id, noodles, bite;
    Table &table;

    void main() {
        osacquire( cout ) << "philosopher " << setw(8) << id << " will eat " << noodles << " noodles" << endl;
        for ( ;; ) {
            table.pickup( id );							// pick up both forks
            yield( rand() % 10 );						// pretend to eat
            bite = rand() % 5 + 1;						// take a bite
            if ( bite >= noodles ) {					// finished eating ?
                osacquire( cout ) << "philosopher:" << id << " finished eating the last " << noodles << endl;
                table.putdown( id );					// make sure the forks are put down
                break;
            } // exit
            noodles -= bite;							// reduce noodles
            osacquire( cout ) << "philosopher:" << id << " is eating " << bite << " noodles leaving " << noodles << " noodles" << endl;
            table.putdown( id );						// put down both forks
            yield( rand() % 10 );						// pretend to think
        } // for
    } // Philosopher::main
  public:
    Philosopher( int id, int noodles, Table &table ) : id( id ), noodles( noodles ), table( table ) {
    } // Philosopher::Philosopher
}; // Philosopher


void uMain::main() {
    int NoOfPhils = 5, NoOfNoodles = 30;

    switch ( argc ) {
      case 3:
        NoOfNoodles = atoi( argv[2] );
      case 2:
        NoOfPhils = atoi( argv[1] );
        break;
      case 1:
        break;
      default:
        cerr << "usage : " << argv[0] << " philosophers noodles" << endl;
        exit( EXIT_FAILURE );
    } // switch

    Table table( NoOfPhils );
    Philosopher *phil[NoOfPhils];
    int i;
    
//	uProcessor processor[NoOfPhils - 1];
    for ( i = 0; i < NoOfPhils; i += 1 ) {				// create Philosophers to eat
        phil[i] = new Philosopher( i, NoOfNoodles, table );
    } // for

    for ( i = 0; i < NoOfPhils; i += 1 ) {				// delete Philosophers after eating
        delete phil[i];
    } // for

    cout << "successful completion" << endl;
} // uMain::main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++ Philosopher.cc" //
// End: //
