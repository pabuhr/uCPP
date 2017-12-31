// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2017
// 
// uDefaultActorMailboxes.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Dec 22 13:35:43 2017
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Dec 22 13:47:48 2017
// Update Count     : 2
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


#include <uDefault.h>


// Must be a separate translation unit so that an application can redefine this routine and the loader does not link
// this routine from the uC++ standard library.


unsigned int uDefaultActorMailboxes() {
    return __U_DEFAULT_ACTOR_MAILBOXES__;		// number of executor request queues (mailboxes)
} // uDefaultActorMailboxes


// Local Variables: //
// compile-command: "make install" //
// End: //
