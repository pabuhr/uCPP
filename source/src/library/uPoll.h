//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uPoll.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Tue Mar 29 17:01:20 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Apr 26 11:17:02 2009
// Update Count     : 30
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


#ifndef __POLL_H__
#define __POLL_H__

#pragma __U_NOT_USER_CODE__


#if defined( __solaris__ )

#define U_FNDELAY O_NONBLOCK				// POSIX non-blocking

// ENOBUFS : system could not allocate an internal buffer. The operation may
// succeed when buffers become available.
#define U_EWOULDBLOCK EWOULDBLOCK

#else

#define U_FNDELAY FNDELAY
#define U_EWOULDBLOCK EWOULDBLOCK

#endif

class uPoll {
  public:
    enum PollStatus { NeverPoll, PollOnDemand, AlwaysPoll };
  private:
    PollStatus uStatus;
  public:
    PollStatus getStatus() {
	return uStatus;
    } // uPoll::getStatus

    void setStatus( PollStatus s ) {
	uStatus = s;
    } // uPoll::setStatus

    void setPollFlag( int fd );
    void clearPollFlag( int fd );
    void computeStatus( int fd );
}; // uPoll


#pragma __U_USER_CODE__

#endif // __POLL_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
