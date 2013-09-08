//                              -*- Mode: C++ -*-
//
// uC++ Version 6.0.0, Copyright (C) Jiongxiong Chen and Ashif S. Harji 2003
//
// uLifoScheduler.h --
//
// Author           : Jiongxiong Chen and Ashif S. Harji
// Created On       : Fri Feb 14 14:26:49 2003
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Dec  5 23:41:00 2012
// Update Count     : 143
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


#ifndef __U_LIFOSCHEDULER_H__
#define __U_LIFOSCHEDULER_H__

#pragma __U_NOT_USER_CODE__

#include <uC++.h>

class uLifoScheduler : public uBaseSchedule<uBaseTaskDL> {
    uBaseTaskSeq list;					// list of tasks awaiting execution
  public:
    bool empty() const;
    void add( uBaseTaskDL *node );
    uBaseTaskDL *drop();
    bool checkPriority( uBaseTaskDL &owner, uBaseTaskDL &calling );
    void resetPriority( uBaseTaskDL &owner, uBaseTaskDL &calling );
    void addInitialize( uBaseTaskSeq &taskList );
    void removeInitialize( uBaseTaskSeq &taskList );
    void rescheduleTask( uBaseTaskDL *taskNode, uBaseTaskSeq &taskList );
}; // uLifoScheduler

#pragma __U_USER_CODE__

#endif //  __U_LIFOSCHEDULER_H__

// Local Variables: //
// compile-command: "make install" //
// End: //
