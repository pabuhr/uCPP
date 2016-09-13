//                              -*- Mode: C++ -*-
//
// uC++ Version 6.1.0, Copyright (C) Philipp E. Lim 1996
//
// uDeadlineMonotonic.cc --
//
// Author           : Philipp E. Lim and Ashif S. Harji
// Created On       : Fri Oct 27 07:29:18 2000
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri May 27 06:25:20 2016
// Update Count     : 43
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
#include <uDeadlineMonotonic.h>

//#include <uDebug.h>


// Compare abstracts the comparison of two task's priorities.  The deadline is first checked.  If the deadlines are
// identical for two tasks, the period or frame field is checked.  Non-real-time tasks are always greater in deadline
// than real-time tasks.  Aperiodic tasks get lowest deadline or priority among all real-time tasks.  This compare
// function acts in the same way as strcmp in terms of return value.

int uDeadlineMonotonic::compare( uBaseTask &task1, uBaseTask &task2 ) {
    uDuration temp;
    enum Codes { PS_PS, PS_NPS, NPS_PS, R_R, R_NR, NR_R, NR_NR }; // T1_T2
    Codes taskCodes[][4] = { { NR_NR,	NR_R,	NPS_PS,	NPS_PS	},
			     { R_NR,	R_R,	NPS_PS,	NPS_PS	},
			     { PS_NPS,  PS_NPS, PS_PS,	PS_PS	},
			     { PS_NPS,  PS_NPS, PS_PS,	PS_PS	} };

    uRealTimeBaseTask *rbtask1, *rbtask2;
    uPeriodicBaseTask *pbtask1, *pbtask2;
    uSporadicBaseTask *sbtask1 = NULL, *sbtask2 = NULL;
    int index1, index2;

    rbtask1 = dynamic_cast<uRealTimeBaseTask *>(&task1);
    if ( rbtask1 == NULL ) {
	index1 = 0;
	pbtask1 = NULL;
	sbtask1 = NULL;
    } else {
	index1 = 1;

	if ( ( pbtask1 = dynamic_cast<uPeriodicBaseTask *>(&task1) ) != NULL ) {
	    index1 = 2;
	} else if ( ( sbtask1 = dynamic_cast<uSporadicBaseTask *>(&task1) ) != NULL ) {
	    index1 = 3;
	} // if
    } // if

    rbtask2 = dynamic_cast<uRealTimeBaseTask *>(&task2);
    if ( rbtask2 == NULL ) {
	index2 = 0;
	pbtask2 = NULL;
	sbtask2 = NULL;
    } else {
	index2 = 1;

	if ( ( pbtask2 = dynamic_cast<uPeriodicBaseTask *>(&task2) ) != NULL ) {
	    index2 = 2;
	} else if ( ( sbtask2 = dynamic_cast<uSporadicBaseTask *>(&task2) ) != NULL ) {
	    index2 = 3;
	} // if
    } // if

    switch ( taskCodes[index1][index2] ){
      case PS_PS:					// both task1 and task2 periodic or sporadic
	temp = rbtask1->getDeadline() - rbtask2->getDeadline();
	if ( temp > 0 ) {
	    return 1;
	} else if ( temp < 0 ) {
	    return -1;
	} else {					// real-time tasks have equal deadlines => check their periods or frames
	    uDuration period1, period2;

	    if ( pbtask1 != NULL ) {			// periodic ?
		period1 = pbtask1->getPeriod();
	    } else if ( sbtask1  != NULL ) {		// sporadic ?
		period1 = sbtask1->getFrame();
	    } else {
		uAbort( "(uDeadlineMonotonic *)%p.compare : internal error.", this );
	    } // if

	    if ( pbtask2 != NULL ) {			// periodic ?
		period2 = pbtask2->getPeriod();
	    } else if ( sbtask2  != NULL ) {		// sporadic ?
		period2 = sbtask2->getFrame();
	    } else {
		uAbort( "(uDeadlineMonotonic *)%p.compare : internal error.", this );
	    } // if

	    temp = period1 - period2;
	    return (temp > 0) ? 1 : (temp < 0) ? -1 : 0;
	} // if
	break;

      case PS_NPS:					// task1 periodic or sporadic and task2 is not ?
	return -1;
	break;

      case NPS_PS:					// task1 is not and task2 periodic or sporadic ?
	return 1;
	break;

      case R_R:						// both task1 and task2 aperiodic
	temp = rbtask1->getDeadline() - rbtask2->getDeadline();
	return (temp > 0) ? 1 : (temp < 0) ? -1 : 0;
	break;

      case R_NR:					// task1 aperiodic and task2 is not ?
	return -1;
	break;

      case NR_R:					// task1 is not and task2 is aperiodic ?
	return 1;
	break;

      case NR_NR:					// both task1 and task1 are non-real-time
	return 0;
	break;

      default:
	uAbort( "(uDeadlineMonotonic *)%p.compare : internal error.", this );
	break;
    } // switch
} // uDeadlineMonotonic::compare


void uDeadlineMonotonic::addInitialize( uSequence<uBaseTaskDL> &taskList ) {
#ifdef __U_DEBUG_H__
    uDebugPrt( "(uDeadlineMonotonic &)%p.addInitialize: enter\n", this );
#endif // __U_DEBUG_H__
    uSequence<uBaseTaskDL> List;
    uSeqIter<uBaseTaskDL> iter;
    int cnt = 0;
    uBaseTaskDL *ref = NULL, *prev = NULL, *node = NULL;

    // The cluster's list of tasks is maintained in sorted order. This algorithm relies on the kernel code adding new
    // tasks to the end of the cluster's list of tasks.  (Perhaps better to combine adding tasks to a cluster list with
    // initializing newly scheduled tasks so only one call is made in uTaskAdd.)

    uRealTimeBaseTask *rtb = dynamic_cast<uRealTimeBaseTask *>(&(taskList.tail()->task()));

    if ( rtb == NULL ) {
#ifdef __U_DEBUG_H__
	uDebugPrt( "(uDeadlineMonotonic &)%p.addInitialize: exit1\n", this );
#endif // __U_DEBUG_H__
	return;
    } // exit

    ref = taskList.dropTail();

    if ( ref == NULL ) {				// necessary if addInitialize is called from removeInitialize
#ifdef __U_DEBUG_H__
	uDebugPrt( "(uDeadlineMonotonic &)%p.addInitialize: exit2\n", this );
#endif // __U_DEBUG_H__
	return;
    } // exit

    for ( iter.over(taskList), prev = NULL; iter >> node ; prev = node ) { // find place in the list to insert
	if ( compare( ref->task(), node->task() ) < 0 ) break;
    } // for
    taskList.insertBef( ref, node );

    // Find out if task was ever on cluster, if so compare verCount with verCount for cluster.  If different or first
    // visit recalculate otherwise, stop after putting task back into task list

    if ( verCount == (unsigned int)rtb->getVersion( uThisCluster() ) ) {
#ifdef __U_DEBUG_H__
	uDebugPrt( "(uDeadlineMonotonic &)%p.addInitialize: exit3\n", this );
#endif // __U_DEBUG_H__
	return;
    } // exit

    // either new task or verCounts differ, increment verCount and continue
    verCount += 1;

    for ( iter.over( taskList ), cnt = 0, prev = taskList.head(); iter >> node; prev = node ) {
	if ( compare( prev->task(), node->task() ) != 0 ) {
	    cnt += 1;
	} // if
	uBaseTask &task = node->task();
	uRealTimeBaseTask *rtask = dynamic_cast<uRealTimeBaseTask *>(&task);
	if ( rtask != NULL ) {
	    setBasePriority( *rtask, cnt );
	    rtask->setVersion( uThisCluster(), verCount );
	    setActivePriority( *rtask, getInheritTask( *rtask ) );
	} else {
	    if ( getBasePriority( task ) == 0 ) {		// if first time, intialize priority for nonreal-time task
		setBasePriority( task, __U_MAX_NUMBER_PRIORITIES__ - 1);
	    } // if
	    setActivePriority( task, getInheritTask( task ) );
	} // if
    } // for

    if ( cnt > __U_MAX_NUMBER_PRIORITIES__ ) {
	uAbort( "(uDeadlineMonotonic &)%p.addInitialize : Cannot schedule task as more priorities are needed than current limit of %d.",
		this, __U_MAX_NUMBER_PRIORITIES__ );
    } // if

    while( ! empty() ) {				// re-arrange ready-queue
	List.insertBef( drop(), NULL );
    } // while
    while( ! List.empty() ) {
	add( List.dropHead() );
    } // while

#ifdef __U_DEBUG_H__
// 	uSeqIter<uBaseTaskDL> j;
//	uBaseTaskDL *ptr = NULL;
//      for (j.over(taskList); j >> ptr; ) {
//          fprintf(stderr, "%p Task with priority %d\n", &(ptr->task()), ptr->task().getActivePriority());
//      }
//      fprintf(stderr, "Leaving uInitialize! \n");
	uDebugPrt( "(uDeadlineMonotonic &)%p.addInitialize: exit4\n", this );
#endif // __U_DEBUG_H__
} // uDeadlineMonotonic::addInitialize


void uDeadlineMonotonic::removeInitialize( uSequence<uBaseTaskDL> & ) {
    // Although removing a task may leave a hole in the priorities, the hole should not affect the ability to schedule
    // the task or the order the tasks execute. Therefore, no rescheduling is performed.

//	addInitialize( taskList );
} // uDeadlineMonotonic::removeInitialize


void uDeadlineMonotonic::rescheduleTask( uBaseTaskDL *taskNode, uBaseTaskSeq &taskList ) {
    verCount += 1;
    taskList.remove( taskNode );
    taskList.addTail( taskNode );
    addInitialize( taskList );
} // uDeadlineMonotonic::rescheduleTask


// Local Variables: //
// compile-command: "make install" //
// End: //
