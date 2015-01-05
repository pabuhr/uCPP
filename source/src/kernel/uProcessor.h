//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Peter A. Buhr 1994
// 
// uProcessor.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu May 26 09:36:12 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Apr  6 13:51:50 2012
// Update Count     : 52
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


#ifndef __U_PROCESSOR_H__
#define __U_PROCESSOR_H__


//######################### uProcessorTask #########################


_Task uProcessorTask {
    friend class uProcessor;				// access: setPreemption

    uProcessor &processor;				// associated processor
    uCondition result;


    unsigned int preemption;				// communication: setPreemption
    uCluster *cluster;					// communication: setCluster

    void main();
    _Mutex void setPreemption( unsigned int ms );
    _Mutex void setCluster( uCluster &cluster );

    uProcessorTask( uCluster &cluster, uProcessor &processor );
    ~uProcessorTask();
  public:
}; // uProcessorTask


#endif // __U_PROCESSOR_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
