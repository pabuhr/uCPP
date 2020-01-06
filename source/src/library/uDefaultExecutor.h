//                               -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1997
// 
// uDefaultExecutor.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Jan  2 20:45:32 2020
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Jan  3 14:08:48 2020
// Update Count     : 5
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


#pragma once


// Define the default number of processors created in the actor executor. Must be greater than 0.

#define __U_DEFAULT_EXECUTOR_PROCESSORS__ 2

// Define the default number of threads created in the actor executor. Must be greater than 0.

#define __U_DEFAULT_EXECUTOR_THREADS__ 2

// Define the default number of executor request-queues (mailboxes) written to by actors and serviced by the
// actor-executor threads. Must be greater than 0.

#define __U_DEFAULT_EXECUTOR_RQUEUES__ 2

// Define if actor executor is created in a separate cluster

#define __U_DEFAULT_EXECUTOR_SEPCLUS__ false

// Define affinity for actor executor kernel threads and the offset from CPU 0 to start binding. -1 implies no affinity.

#define __U_DEFAULT_EXECUTOR_AFFINITY__ -1


extern unsigned int uDefaultExecutorProcessors();	// kernel threads (processors) servicing executor thread-pool
extern unsigned int uDefaultExecutorThreads();		// user threads servicing executor thread-pool
extern unsigned int uDefaultExecutorRQueues();		// executor request queues
extern bool uDefaultExecutorSepClus();			// create processors on separate cluster
extern int uDefaultExecutorAffinity();			// affinity and offset (-1 => no affinity, default)

