//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 2010
// 
// DeleteProcessor.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sun Jul 18 11:03:54 2010
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Aug 18 21:30:26 2012
// Update Count     : 3
// 

#include <iostream>
using namespace std;

_Task worker {
    void main() {
	migrate( clus );
	cout << "Task " << &uThisTask() << " deleting processor " << &uThisProcessor() << "\n";
	delete &uThisProcessor();
	cout << "Task " << &uThisTask() << " now on processor " << &uThisProcessor() << "\n";
	yield();
	cout << "Task " << &uThisTask() << " still on processor " << &uThisProcessor() << "\n";
    }

    uCluster &clus;
  public:
    worker( uCluster &cl ): clus( cl ) {}
}; // worker

void uMain::main(){
    uCluster cluster;
    uProcessor *processor = new uProcessor( cluster );
    cout << "Task main created processor " << processor << "\n";
    {
	worker f( cluster );
	uSleep( uDuration( 2 ) );
	processor = new uProcessor( cluster );
	cout << "Task main created processor " << processor << "\n";
    }
    delete processor;
    cout << "successful completion\n";
}
