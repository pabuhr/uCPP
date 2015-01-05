//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// Migrate.cc -- Test migrating of tasks among clusters.
// 
// Author           : Peter A. Buhr
// Created On       : Tue Jun 28 10:23:11 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Jul 24 17:44:12 2012
// Update Count     : 101
// 

#include <iostream>
using namespace std;

unsigned int uDefaultPreemption() {
    return 1;
} // uDefaultPreemption

unsigned int uDefaultSpin() {
    return 10;						// keep small for 1-core computer
} // uDefaultSpin

const unsigned int NoOfClusters = 3;

_Task Worker {
    int id;
    uCluster **clusters;
    char name[20];

    void main() {
	const int unsigned NoOfTimes = 10000;
	sprintf( name, "Worker%d", id );
	setName( name );

        for ( unsigned int i = 0; i < NoOfTimes; i += 1 ) {
	    for ( unsigned int j = 0; j < NoOfClusters; j += 1, id = (id + 1) % NoOfClusters ) {
		migrate( *(clusters[id]) );
	    } // for
        } // for
    } // Worker::main
  public:
    Worker( int id, uCluster &create, uCluster *clusters[] ) : uBaseTask( create ), id( id ), clusters( clusters ) {
    } // Worker::Worker
}; // Worker


void uMain::main() {
    const unsigned int NoOfTasks = 3;

    uCluster *clusters[NoOfClusters];
    uProcessor *processors[NoOfClusters];
    Worker *tasks[NoOfTasks];
    unsigned int i, j;

    clusters[0] = &uThisCluster();			// default cluster and processor
    processors[0] = &uThisProcessor();

    for ( i = 1; i < NoOfClusters; i += 1 ) {		// create N-1 clusters and processors
	clusters[i] = new uCluster();
	processors[i] = new uProcessor( *(clusters[i]) );
    } // for

    for ( i = 0, j = 0; i < NoOfTasks; i += 1, j = (j + 1) % NoOfClusters ) { // create tasks
	tasks[i] = new Worker( j, *(clusters[j]), clusters );
    } // for
    for ( i = 0; i < NoOfTasks; i += 1 ) {		// delete tasks
	delete tasks[i];
    } // for

    for ( i = 1; i < NoOfClusters; i += 1 ) {		// delete N-1 clusters and processors
	delete processors[i];
	delete clusters[i];
    } // for

    cout << "successful completion" << endl;
} // uMain::main

// Local Variables: //
// compile-command: "u++ Migrate.cc" //
// End: //
