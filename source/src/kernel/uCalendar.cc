//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Philipp E. Lim 1996
// 
// uCalendar.cc -- 
// 
// Author           : Philipp E. Lim
// Created On       : Thu Jan 11 08:23:17 1996
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Sep 14 13:27:33 2007
// Update Count     : 202
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
//#include <uDebug.h>

#include <ostream>


//######################### uDuration #########################


std::ostream &operator<<( std::ostream &os, const uDuration op ) {
    os << op.tv / TIMEGRAN << ".";
    os.width(9);					// nanoseconds
    char oc = os.fill( '0' );
    os << ( op.tv < 0 ? -op.tv : op.tv ) % TIMEGRAN;
    os.fill( oc );
    return os;
} // operator<<


//######################### uTime #########################


std::ostream &operator<<( std::ostream &os, const uTime op ) {
    os << op.tv / TIMEGRAN << ".";
    os.width(9);					// nanoseconds
    char oc = os.fill( '0' );
    os << ( op.tv < 0 ? -op.tv : op.tv ) % TIMEGRAN;
    os.fill( oc );
    return os;
} // operator<<


#ifdef __U_DEBUG__
const char *uTime::uCreateFmt = "Attempt to create uTime( year=%d, month=%d, day=%d, hour=%d, min=%d, sec=%d, nsec=%d ), "
				"which exceeds range 00:00:00 UTC, January 1, 1970 to 03:14:07 UTC, January 19, 2038.";
#endif // __U_DEBUG__


void uTime::uCreateTime( int year, int month, int day, int hour, int min, int sec, long int nsec ) {
    tm t;

    tzset();						// initialize time global variables
    t.tm_isdst = -1;					// let mktime determine if alternate timezone is in effect
    t.tm_year = year - 1900;				// mktime uses 1900 as its starting point
    t.tm_mon = month;
    t.tm_mday = day + 1;				// mktime uses range 1-31
    t.tm_hour = hour;
    t.tm_min = min;
#if defined( __freebsd__ )
    t.tm_sec = sec;
    time_t epochsec = timegm( &t );			// get GMT
#else
    t.tm_sec = sec - ::timezone;			// adjust off the timezone (global variable!) to get GMT
    time_t epochsec = mktime( &t );
#endif // __freebsd__
#ifdef __U_DEBUG__
    if ( epochsec == (time_t)-1 ) {
	uAbort( uCreateFmt, year, month, day, hour, min, sec, nsec );
    } // if
#endif // __U_DEBUG__
    tv = (long long int)(epochsec) * TIMEGRAN + nsec;	// convert to nanoseconds
#ifdef __U_DEBUG__
    if ( tv > 2147483647LL * TIMEGRAN ) {		// between 00:00:00 UTC, January 1, 1970 and 03:14:07 UTC, January 19, 2038.
	uAbort( uCreateFmt, year, month, day, hour, min, sec, nsec );
    } // if
#endif // __U_DEBUG__
} // uTime::uCreateTime


//######################### uClock #########################


// uClock::uClock( int ) {
//     // Use exceptions here later on, to see if clock call works
//     //		clock_gettime( CLOCK_REALTIME, &curr );
//     // Right now, only support one real clock.  Later, appropriately set clocktype
//     //		clocktype=clock_id;
//     clocktype = CLOCK_REALTIME;
// } // uClock::uClock


void uClock::resetClock( uTime adj ) {
#if defined( REALTIME_POSIX )
    timespec curr;
    clock_gettime( CLOCK_REALTIME, &curr );
#else
    timeval curr;
    GETTIMEOFDAY( &curr );
#endif
    uTime currtime( curr.tv_sec, curr.tv_usec * 1000 );	// convert to nanoseconds
    clocktype = -1;
    offset.tv = currtime.tv - adj.tv;
} // uClock::resetClock


uTime uClock::getTime() {				// ##### REFERENCED IN TRANSLATOR #####
#if defined( REALTIME_POSIX )
    timespec curr;
    if ( clocktype < 0 ) type = CLOCK_REALTIME;
    clock_gettime( type, &curr );
#else
    timeval curr;
    GETTIMEOFDAY( &curr );
#endif
    uTime currtime( curr.tv_sec, curr.tv_usec * 1000 );	// convert to nanoseconds

    if ( clocktype < 0 ) {				// using virtual clock if < 0
	currtime.tv -= offset.tv;			// adjust the time to reflect the "virtual" time.
    } // if

    return currtime;
} // uClock::getTime


void uClock::getTime( int &year, int &month, int &day, int &hour, int &min, int &sec, long int &nsec ) {
    const timeval temp = getTime();
    tm t;
    localtime_r( (const time_t *)&temp.tv_sec, &t );
    year = t.tm_year; month = t.tm_mon; day = t.tm_mday; hour = t.tm_hour; min = t.tm_min; sec = t.tm_sec;
    nsec = temp.tv_XSEC;
} // uClock::getTime


void uClock::convertTime( uTime time, int &year, int &month, int &day, int &hour, int &min, int &sec, long int &nsec ) {
    const timeval temp = time;
    tm t;
    localtime_r( (const time_t *)&temp.tv_sec, &t );
    year = t.tm_year; month = t.tm_mon; day = t.tm_mday; hour = t.tm_hour; min = t.tm_min; sec = t.tm_sec;
    nsec = temp.tv_XSEC;
} // uClock::convertTime


// Local Variables: //
// compile-command: "make install" //
// End: //
