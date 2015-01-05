//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 2012
// 
// TimeoutProcessor.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Apr  5 08:06:57 2012
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Apr  5 08:07:40 2012
// Update Count     : 2
// 

#include <iostream>
using namespace std;

// The processor is removed from cluster so task "t" cannot run after the timeout moves it to the ready queue on cluster
// "clus". When the processor is returned to "clus", the task "t" can wakeup from the sleep and continue.

_Task T {
    void main() {
	osacquire( cout ) << "start " << uThisCluster().getName() << " " << &uThisProcessor() << endl;
	sleep( 5 );
	osacquire( cout ) << "done " << uThisCluster().getName() << " " << &uThisProcessor() << endl;
    }
  public:
    T( uCluster &clus ) : uBaseTask( clus ) {}
};

void uMain::main() {
    uCluster clus( "clus" );
    osacquire( cout ) << "cluster: " << &clus << endl;
    uProcessor p;
    T t( clus );
    osacquire( cout ) << "over" << endl;
    p.setCluster( clus );
    sleep( 1 );
    osacquire( cout ) << "back" << endl;
    p.setCluster( uThisCluster() );
    sleep( 10 );
    osacquire( cout ) << "over" << endl;
    p.setCluster( clus );
    osacquire( cout ) << "finish" << endl;
}

// Local Variables: //
// compile-command: "u++ TimeoutProcessor.cc" //
// End: //
