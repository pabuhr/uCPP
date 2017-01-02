//                              -*- Mode: C++ -*-
//
// uC++ Version 7.0.0, Copyright (C) Philipp E. Lim 1996
//
// uDeadlineMonotonic.h --
//
// Author           : Philipp E. Lim and Ashif S. Harji
// Created On       : Fri Jul 19 23:20:17 1996
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri May 13 22:17:49 2011
// Update Count     : 133
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


#ifndef __U_DEADLINEMONOTONIC_H__
#define __U_DEADLINEMONOTONIC_H__

#pragma __U_NOT_USER_CODE__


#include <uRealTime.h>


class uDeadlineMonotonic : public uPriorityScheduleSeq<uBaseTaskSeq, uBaseTaskDL> {
    int compare( uBaseTask &task1, uBaseTask &task2 );
  public:
    void addInitialize( uSequence<uBaseTaskDL> &taskList );
    void removeInitialize( uSequence<uBaseTaskDL> & );
    void rescheduleTask( uBaseTaskDL *taskNode, uBaseTaskSeq &taskList );
}; // uDeadlineMonotonic


#pragma __U_USER_CODE__

#endif // __U_DEADLINEMONOTONIC_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
