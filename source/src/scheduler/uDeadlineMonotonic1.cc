//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Philipp E. Lim 1996
//
// uDeadlineMonotonic1.cc --
//
// Author           : Philipp E. Lim and Ashif S. Harji
// Created On       : Fri Oct 27 08:25:33 2000
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri May 13 22:14:12 2011
// Update Count     : 33
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
#include <uDeadlineMonotonic1.h>

//#include <uDebug.h>


void uDeadlineMonotonic1::addInitialize( uSequence<uBaseTaskDL> &taskList ) {
#ifdef __U_DEBUG_H__
    uDebugPrt( "(uDeadlineMonotonic1 &)%p.addInitialize: enter\n", this );
#endif // __U_DEBUG_H__

    uBaseTask &task = taskList.tail()->task();

    uPIHeap *PIHptr = dynamic_cast<uPIHeap *>(task.uPIQ);
    if ( PIHptr == NULL ) {
	uAbort("(uDeadlineMonotonic1 &)%p.addInitialize : Task %p has incorrect uPIQ type.", this, &task );
    } // if

    int queueNum = PIHptr->head();
    int priority = PIHptr->getHighestPriority();
    uRealTimeBaseTask *rbtask;
    uPeriodicBaseTask *pbtask;
    uSporadicBaseTask *sbtask;

    if ( ( rbtask = dynamic_cast<uRealTimeBaseTask *>(&task) ) == NULL ) {
#ifdef __U_DEBUG_H__
	uDebugPrt( "(uDeadlineMonotonic1 &)%p.addInitialize: exit1\n", this );
#endif // __U_DEBUG_H__
	setBasePriority( task, INT_MAX );		// set to a large number

	if ( queueNum == -1 ) {
	    setActivePriority( task, task );
	} else {
	    setActivePriority( task, priority );
	} // if
    } else if ( ( pbtask = dynamic_cast<uPeriodicBaseTask *>(&task) ) != NULL ) {
	setBasePriority( task, (int)(pbtask->getPeriod().nanoseconds() / 1000000) );
	if ( queueNum == -1 ) {
	    setActivePriority( *pbtask, task );
	} else {
	    setActivePriority( *pbtask, priority );
	} // if
    } else if ( ( sbtask = dynamic_cast<uSporadicBaseTask *>(&task) ) != NULL ) {
	setBasePriority( task, (int)(sbtask->getFrame().nanoseconds() / 1000000) );
	if ( queueNum == -1 ) {
	    setActivePriority( *sbtask, task );
	} else {
	    setActivePriority( *sbtask, priority );
	} // if
    } else {						// only uRealtime
	setBasePriority( task, (int)(rbtask->getDeadline().nanoseconds() / 1000000) );
	if ( queueNum == -1 ) {
	    setActivePriority( (uRealTimeBaseTask &)task, task );
	} else {
	    setActivePriority( (uRealTimeBaseTask &)task, priority );
	} // if
    } // if

    // must assign a queue if none already assigned increment count.  do linear
    // search checking priority values.
    int tpri = getBasePriority( task );
    bool flag = false;

    for ( int i = 0; i < num_priorities; i += 1 ) {
	if ( tpri == objects[i].priority ) {
	    flag = true;
	    setBaseQueue( task, i);
	    if ( queueNum == -1 ) {
		setActiveQueue( task, i );		// should have at least t's serial on queue by now
	    } else {
		setActiveQueue( task, queueNum );	// should have at least t's serial on queue by now
	    } // if
	    break;
	} // if
    } // for

    if ( ! flag ) {
	num_priorities += 1;
	if ( num_priorities <= __U_MAX_NUMBER_PRIORITIES__ ) {
	    objects[num_priorities - 1].priority = tpri;
	    setBaseQueue( task, num_priorities - 1 );
	    if ( queueNum == -1 ) {
		setActiveQueue( task, num_priorities - 1 ); // should have at least t's serial on queue by now
	    } else {
		setActiveQueue( task, queueNum );
	    } // if
	} else {
	    uAbort( "(uDeadlineMonotonic1 &)%p.addInitialize : Cannot schedule task as more priorities are needed than current limit of %d.",
		    this, __U_MAX_NUMBER_PRIORITIES__ );
	} // if
    } // if
} // uDeadlineMonotonic1::addInitialize


void uDeadlineMonotonic1::removeInitialize( uSequence<uBaseTaskDL> & ) {
    // Although removing a task may leave a hole in the priorities, the hole
    // should not affect the ability to schedule the task or the order the
    // tasks execute. Therefore, no rescheduling is performed.

//	addInitialize( taskList );
} // uDeadlineMonotonic1::removeInitialize


void uDeadlineMonotonic1::rescheduleTask( uBaseTaskDL *taskNode, uBaseTaskSeq &taskList ) {
    //verCount += 1;
    taskList.remove( taskNode );
    taskList.addTail( taskNode );
    addInitialize( taskList );
} // uDeadlineMonotonic1::rescheduleTask


// Local Variables: //
// compile-command: "make install" //
// End: //
