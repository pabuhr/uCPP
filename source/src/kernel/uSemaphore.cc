//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2003
// 
// uSemaphore.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Nov 20 17:17:52 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Jul 18 07:37:45 2017
// Update Count     : 120
// 
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


#define __U_KERNEL__
#include <uC++.h>

namespace UPP {
//######################### TimedWaitHandler #########################


    uSemaphore::TimedWaitHandler::TimedWaitHandler( uBaseTask &task, uSemaphore &semaphore ) : semaphore( semaphore ) {
	This = &task;
	timedout = false;
    } // uSemaphore::TimedWaitHandler::TimedWaitHandler

    uSemaphore::TimedWaitHandler::TimedWaitHandler( uSemaphore &semaphore ) : semaphore( semaphore ) {
	This = nullptr;
	timedout = false;
    } // uSemaphore::TimedWaitHandler::TimedWaitHandler

    void uSemaphore::TimedWaitHandler::handler() {
	semaphore.waitTimeout( *this );
    } // uSemaphore::TimedWaitHandler::handler


//######################### uSemaphore #########################


    void uSemaphore::waitTimeout( TimedWaitHandler &h ) {
	// This uSemaphore member is called from the kernel, and therefore, cannot block, but it can spin.

	spinLock.acquire();
	uBaseTask &task = *h.getThis();			// optimization
	if ( task.entryRef.listed() ) {			// is task on queue
	    // Remove is a linear search on a queue, but timeouts should be rare and the waiting queue should be short.
	    waiting.remove( &(task.entryRef) );		// remove this task, O(N)
	    h.timedout = true;
	    count += 1;					// adjust the count to reflect the wake up
	    spinLock.release();
	    task.wake();				// wake up task
	} else {
	    spinLock.release();
	} // if
    } // uSemaphore::waitTimeout


    void uSemaphore::P() {				// wait on a semaphore
	spinLock.acquire();
	count -= 1;
	if ( count < 0 ) {
	    waiting.addTail( &(uThisTask().entryRef) );	// queue current task
#ifdef __U_STATISTICS__
	    uFetchAdd( UPP::Statistics::io_lock_queue, 1 );
#endif // __U_STATISTICS__
	    uProcessorKernel::schedule( &spinLock );	// atomically release spin lock and block
	} else {
	    spinLock.release();
	} // if
    } // uSemaphore::P


    bool uSemaphore::P( uDuration duration ) {		// wait on a semaphore
	return P( uThisProcessor().getClock().getTime() + duration );
    } // uSemaphore::P


    bool uSemaphore::P( uTime time ) {			// wait on a semaphore
	spinLock.acquire();
	count -= 1;
	if ( count < 0 ) {
	    uBaseTask &task = uThisTask();		// optimization
	    TimedWaitHandler handler( task, *this );	// handler to wake up blocking task
	    uEventNode timeoutEvent( task, handler, time, 0 );
	    timeoutEvent.executeLocked = true;
	    timeoutEvent.add();
	    waiting.addTail( &(task.entryRef) );	// queue current task
	    uProcessorKernel::schedule( &spinLock );	// atomically release spin lock and block
	    // count is incremented in waitTimeout for timeout
	    timeoutEvent.remove();
	    return ! handler.timedout;
	} else {
	    spinLock.release();
	    return true;
	} // if
    } // uSemaphore::P


    void uSemaphore::P( uSemaphore &s ) {		// wait on a semaphore and release another
	spinLock.acquire();
	if ( &s == this ) {				// perform operation on self ?
	    if ( count < 0 ) {				// V my semaphore
		waiting.dropHead()->task().wake();	// remove task at head of waiting list
	    } // if
	    count += 1;
	} else {
	    s.V();					// V other semaphore
	} // if

	count -= 1;					// now P my semaphore
	if ( count < 0 ) {
	    waiting.addTail( &(uThisTask().entryRef) );	// block current task
	    uProcessorKernel::schedule( &spinLock );	// atomically release spin lock and block
	} else {
	    spinLock.release();
	} // if
    } // uSemaphore::P


    bool uSemaphore::P( uSemaphore &s, uDuration duration ) { // wait on semaphore and release another
	return P( s, uThisProcessor().getClock().getTime() + duration );
    } // uSemaphore::P


    bool uSemaphore::P( uSemaphore &s, uTime time ) {	// wait on semaphore and release another
	spinLock.acquire();
	if ( &s == this ) {				// perform operation on self ?
	    if ( count < 0 ) {				// V my semaphore
		waiting.dropHead()->task().wake();	// remove task at head of waiting list
	    } // if
	    count += 1;
	} else {
	    s.V();					// V other semaphore
	} // if

	count -= 1;					// now P my semaphore
	if ( count < 0 ) {
	    uBaseTask &task = uThisTask();		// optimization
	    TimedWaitHandler handler( task, *this );	// handler to wake up blocking task
	    uEventNode timeoutEvent( task, handler, time, 0 );
	    timeoutEvent.executeLocked = true;
	    timeoutEvent.add();
	    waiting.addTail( &(task.entryRef) );	// queue current task
	    uProcessorKernel::schedule( &spinLock );	// atomically release spin lock and block
	    // count is incremented in waitTimeout for timeout
	    timeoutEvent.remove();
	    return ! handler.timedout;
	} else {
	    spinLock.release();
	    return true;
	} // if
    } //  uSemaphore::P


    bool uSemaphore::TryP() {				// conditionally wait on a semaphore
	spinLock.acquire();
	if ( count > 0 ) {
	    count -= 1;
	    spinLock.release();
	    return true;
	} // if
	spinLock.release();
	return false;
    } // uSemaphore::TryP


    void uSemaphore::V() {				// signal semaphore
	// special form to handle the case where the woken task deletes the semaphore storage
	uBaseTaskDL *task;
	spinLock.acquire();
	count += 1;
	if ( count <= 0 ) {
	    task = waiting.dropHead();			// remove task at head of waiting list
#ifdef __U_STATISTICS__
	    uFetchAdd( UPP::Statistics::io_lock_queue, -1 );
#endif // __U_STATISTICS__
	    spinLock.release();
	    task->task().wake();			// make new owner
	} else {
	    spinLock.release();
	} // if
    } // uSemaphore::V


    void uSemaphore::V( int inc ) {			// signal semaphore
#ifdef __U_DEBUG__
	if ( inc < 0 ) {
	    abort( "Attempt to advance uSemaphore %p to %d that must be >= 0.", this, inc );
	} // if
#endif // __U_DEBUG__
	spinLock.acquire();
	for ( int i = inc; i > 0; i -= 1 ) {
	    if ( count >= 0 ) {
		count += i;
		break;
	    } // if
	    count += 1;
	    waiting.dropHead()->task().wake();		// remove task at head of waiting list and make new owner
	} // for
	spinLock.release();
    } // uSemaphore::V
} // UPP

// Local Variables: //
// compile-command: "make install" //
// End: //
