//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uSemaphore.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Tue Mar 29 13:42:33 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon May 19 22:54:59 2008
// Update Count     : 76
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


#ifndef __U_SEMAPHORE_H__
#define __U_SEMAPHORE_H__

#pragma __U_NOT_USER_CODE__

#ifndef __U_SEMAPHORE_MONITOR__

using UPP::uSemaphore;

#else

_Monitor uSemaphore {
    int count;						// semaphore counter
    uCondition blockedTasks;
  public:
    uSemaphore( unsigned int count = 1 ) : count( count ) {
    } // uSemaphore::uSemaphore

    void P() {						// wait on a semaphore
	count -= 1;					// decrement semaphore counter
	if ( count < 0 ) blockedTasks.wait();		// if semaphore less than zero, wait for next V
    } // uSemaphore::P

    // This routine forces the implementation to use internal scheduling, otherwise there is a deadlock problem with
    // accepting V in the previous P routine, which prevents calls entering this routine to V the parameter and wait.
    // Essentially, it is necessary to enter the monitor and do some work *before* possibly blocking. To use external
    // scheduling requires accepting either the V routine OR this P routine, which is currently impossible because there
    // is no differentiation between overloaded routines. I'm not sure the external scheduling solution would be any
    // more efficient than the internal scheduling solution.

    void P( uSemaphore &s ) {				// wait on a semaphore and release another
	s.V();						// release other semaphore
	P();						// wait
    } // uSemaphore::P

    bool TryP() {					// conditionally wait on a semaphore
      if ( count > 0 ) {
	    count -= 1;					// decrement semaphore counter
	    return true;
	};
	return false;
    } // uSemaphore::TryP

    void V( unsigned int times = 1 ) {			// signal a semaphore
	count += times;					// increment semaphore counter
	for ( unsigned int i = 0; i < times; i += 1 ) {	// wake up required number of tasks
	    blockedTasks.signal();
	} // for
    } // uSemaphore::V

    _Nomutex int counter() const {			// semaphore counter
	return count;
    } // uSemaphore::counter

    _Nomutex bool empty() const {			// no tasks waiting on semaphore ?
	return count >= 0;
    } // uSemaphore::empty
}; // uSemaphore

#endif // ! __U_SEMAPHORE_MONITOR__


#pragma __U_USER_CODE__

#endif // __U_SEMAPHORE_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
