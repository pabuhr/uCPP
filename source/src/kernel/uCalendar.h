//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Philipp E. Lim 1995
// 
// uCalendar.h -- 
// 
// Author           : Philipp E. Lim
// Created On       : Tue Dec 19 11:58:22 1995
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Aug 22 21:46:47 2017
// Update Count     : 252
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


#ifndef __U_CALENDAR_H__
#define __U_CALENDAR_H__


//#if defined( __linux__ )
//#define __need_timespec				// force definitions for timespec
//#include <time.h>					// TEMPORARY: should not be needed
//#endif // __linux__

#include <ctime>
#include <sys/time.h>
#include <iosfwd>


#define CLOCKGRAN 15000000L				// ALWAYS in nanoseconds, MUST BE less than 1 second
#define TIMEGRAN 1000000000L				// nanosecond granularity, except for timeval
#define GETTIMEOFDAY( tp ) gettimeofday( (tp), (struct timezone *)0 )


#if defined( REALTIME_POSIX )
#define tv_XSEC tv_nsec
#else
#define tv_XSEC tv_usec
#endif


#if defined( __linux__ )
// fake a few things
#define	CLOCK_REALTIME	0				// real (clock on the wall) time
#endif


class uDuration;					// forward declaration
class uTime;						// forward declaration
class uClock;						// forward declaration


//######################### uDuration #########################


uDuration operator+( uDuration op );			// forward declaration
uDuration operator+( uDuration op1, uDuration op2 );	// forward declaration
uDuration operator-( uDuration op );			// forward declaration
uDuration operator-( uDuration op1, uDuration op2 );	// forward declaration
uDuration operator*( uDuration op1, long long int op2 ); // forward declaration
uDuration operator*( long long int op1, uDuration op2 ); // forward declaration
uDuration operator/( uDuration op1, long long int op2 ); // forward declaration
long long int operator/( uDuration op1, uDuration op2 ); // forward declaration
bool operator==( uDuration op1, uDuration op2 );	// forward declaration
bool operator!=( uDuration op1, uDuration op2 );	// forward declaration
bool operator>( uDuration op1, uDuration op2 );		// forward declaration
bool operator<( uDuration op1, uDuration op2 );		// forward declaration
bool operator>=( uDuration op1, uDuration op2 );	// forward declaration
bool operator<=( uDuration op1, uDuration op2 );	// forward declaration
std::ostream &operator<<( std::ostream &os, const uDuration op ); // forward declaration


//######################### uTime #########################


uTime operator+( uTime op1, uDuration op2 );		// forward declaration
uTime operator+( uDuration op1, uTime op2 );		// forward declaration
uDuration operator-( uTime op1, uTime op2 );		// forward declaration
uTime operator-( uTime op1, uDuration op2 );		// forward declaration
bool operator==( uTime op1, uTime op2 );		// forward declaration
bool operator!=( uTime op1, uTime op2 );		// forward declaration
bool operator>( uTime op1, uTime op2 );			// forward declaration
bool operator<( uTime op1, uTime op2 );			// forward declaration
bool operator>=( uTime op1, uTime op2 );		// forward declaration
bool operator<=( uTime op1, uTime op2 );		// forward declaration
std::ostream &operator<<( std::ostream &os, const uTime op ); // forward declaration


//######################### uDuration (cont) #########################


class uDuration {
    friend class uTime;
    friend class uClock;
    friend uDuration operator+( uDuration op1 );
    friend uDuration operator+( uDuration op1, uDuration op2 );
    friend uDuration operator-( uDuration op );
    friend uDuration operator-( uDuration op1, uDuration op2 );
    friend uDuration operator*( uDuration op1, long long int op2 );
    friend uDuration operator/( uDuration op1, long long int op2 );
    friend long long int operator/( uDuration op1, uDuration op2 );
    friend long long int operator%( uDuration op1, uDuration op2 );
    friend bool operator==( uDuration op1, uDuration op2 );
    friend bool operator!=( uDuration op1, uDuration op2 );
    friend bool operator>( uDuration op1, uDuration op2 );
    friend bool operator<( uDuration op1, uDuration op2 );
    friend bool operator>=( uDuration op1, uDuration op2 ); 
    friend bool operator<=( uDuration op1, uDuration op2 );
    friend uDuration abs( uDuration op );
    friend std::ostream &operator<<( std::ostream &os, const uDuration op );

    friend uDuration operator-( uTime op1, uTime op2 );
    friend uTime operator-( uTime op1, uDuration op2 );
    friend uTime operator+( uTime op1, uDuration op2 );

    long long int tv;
  public:
    uDuration() {
    } // uDuration::uDuration

    uDuration( long int sec ) {
	tv = (long long int)sec * TIMEGRAN;
    } // uDuration::uDuration

    uDuration( long int sec, long int nsec ) {
	tv = (long long int)sec * TIMEGRAN + nsec;
    } // uDuration::uDuration

    uDuration( const timeval &t ) {
	tv = (long long int)t.tv_sec * TIMEGRAN + t.tv_usec * 1000;
    } // uDuration::uDuration

    uDuration( const timespec &t ) {
	tv = (long long int)t.tv_sec * TIMEGRAN + t.tv_nsec;
    } // uDuration::uDuration

    uDuration &operator=( const timeval &t ) {
	tv = (long long int)t.tv_sec * TIMEGRAN + t.tv_usec * 1000;
	return *this;
    } // uDuration::operator=

    uDuration &operator=( const timespec &t ) {
	tv = (long long int)t.tv_sec * TIMEGRAN + t.tv_nsec;
	return *this;
    } // uDuration::operator=

    operator timeval() const {
	timeval dummy;
	dummy.tv_sec = tv / TIMEGRAN;			// seconds
	dummy.tv_usec = tv % TIMEGRAN / ( TIMEGRAN / 1000000L ); // microseconds
	return dummy;
    } // uDuration::operator timeval

    operator timespec() const {
	timespec dummy;
	dummy.tv_sec = tv / TIMEGRAN;			// seconds
	dummy.tv_nsec = tv % TIMEGRAN;			// nanoseconds
	return dummy;
    } // uDuration::operator timespec

    long long int nanoseconds() const {
	return tv;
    } // uDuration::nanoseconds

    uDuration &operator-=( uDuration op ) {
	*this = *this - op;
	return *this;
    } // uDuration::operator-=

    uDuration &operator+=( uDuration op ) {
	*this = *this + op;
	return *this;
    } // uDuration::operator+=

    uDuration &operator*=( long long int op ) {
	*this = *this * op;
	return *this;
    } // uDuration::operator*=

    uDuration &operator/=( long long int op ) {
	*this = *this / op;
	return *this;
    } // uDuration::operator/=
}; // uDuration


inline uDuration operator+( uDuration op ) {		// unary
    uDuration ans;
    ans.tv = +op.tv;
    return ans;
} // operator+

inline uDuration operator+( uDuration op1, uDuration op2 ) { // binary
    uDuration ans;
    ans.tv = op1.tv + op2.tv;
    return ans;
} // operator+

inline uDuration operator-( uDuration op ) {		// unary
    uDuration ans;
    ans.tv = -op.tv;
    return ans;
} // operator-

inline uDuration operator-( uDuration op1, uDuration op2 ) { // binary
    uDuration ans;
    ans.tv = op1.tv - op2.tv;
    return ans;
} // operator-

inline uDuration operator*( uDuration op1, long long int op2 ) {
    uDuration ans;
    ans.tv = op1.tv * op2;
    return ans;
} // operator*

inline uDuration operator*( long long int op1, uDuration op2 ) {
    return op2 * op1;
} // operator*

inline uDuration operator/( uDuration op1, long long int op2 ) {
    uDuration ans;
    ans.tv = op1.tv / op2;
    return ans;
} // operator/

inline long long int operator/( uDuration op1, uDuration op2 ) {
    return op1.tv / op2.tv;
} // operator/

inline bool operator==( uDuration op1, uDuration op2 ) {
    return op1.tv == op2.tv;
} // operator==

inline bool operator!=( uDuration op1, uDuration op2 ) {
    return op1.tv != op2.tv;
} // operator!=

inline bool operator>( uDuration op1, uDuration op2 ) {
    return op1.tv > op2.tv;
} // operator>

inline bool operator<( uDuration op1, uDuration op2 ) {
    return op1.tv < op2.tv;
} // operator<

inline bool operator>=( uDuration op1, uDuration op2 ) { 
    return op1.tv >= op2.tv;
} // operator>=

inline bool operator<=( uDuration op1, uDuration op2 ) {
    return op1.tv <= op2.tv;
} // operator<=

inline long long int operator%( uDuration op1, uDuration op2 ) {
    return op1.tv % op2.tv;
} // operator%

inline uDuration abs( uDuration op1 ) {
    if ( op1.tv < 0 ) op1.tv = -op1.tv;
    return op1;
} // abs


//######################### uTime (cont) #########################


class uTime {
    friend class uDuration;
    friend class uClock;
    friend uTime operator+( uTime op1, uDuration op2 );
    friend uDuration operator-( uTime op1, uTime op2 );
    friend uTime operator-( uTime op1, uDuration op2 );
    friend bool operator==( uTime op1, uTime op2 );
    friend bool operator!=( uTime op1, uTime op2 );
    friend bool operator>( uTime op1, uTime op2 );
    friend bool operator<( uTime op1, uTime op2 );
    friend bool operator>=( uTime op1, uTime op2 ); 
    friend bool operator<=( uTime op1, uTime op2 );
    friend std::ostream &operator<<( std::ostream &os, const uTime op );

#ifdef __U_DEBUG__
    static const char *uCreateFmt;
#endif // __U_DEBUG__

    long long int tv;					// gcc specific

    void uCreateTime( int year, int month, int day, int hour, int min, int sec, long int nsec );
  public:
    uTime() {
    } // uTime::uTime

    // These two constructors must not call uCreateTime because of its call to mktime, which subsequently calls
    // malloc. The malloc calls lead to recursion problems because uTime values are created from the sigalrm handler in
    // composing the next context switch event.

    explicit uTime( long int sec ) {			// explicit => unambiguous with uDuration( long int sec )
	tv = (long long int)sec * TIMEGRAN;
#ifdef __U_DEBUG__
	if ( tv < 0 || tv > 2147483647LL * TIMEGRAN ) {	// between 00:00:00 UTC, January 1, 1970 and 03:14:07 UTC, January 19, 2038.
	    abort( uCreateFmt, 1970, 0, 0, 0, 0, sec, 0 );
	} // if
#endif // __U_DEBUG__
    } // uTime::uTime

    uTime( long int sec, long int nsec ) {
	tv = (long long int)sec * TIMEGRAN + nsec;
#ifdef __U_DEBUG__
	if ( tv < 0 || tv > 2147483647LL * TIMEGRAN ) {	// between 00:00:00 UTC, January 1, 1970 and 03:14:07 UTC, January 19, 2038.
	    abort( uCreateFmt, 1970, 0, 0, 0, 0, sec, nsec );
	} // if
#endif // __U_DEBUG__
    } // uTime::uTime

    uTime( int min, int sec, long int nsec ) {
	uCreateTime( 1970, 0, 0, 0, min, sec, nsec );
    } // uTime::uTime

    uTime( int hour, int min, int sec, long int nsec ) {
	uCreateTime( 1970, 0, 0, hour, min, sec, nsec );
    } // uTime::uTime

    uTime( int day, int hour, int min, int sec, long int nsec ) {
	uCreateTime( 1970, 0, day, hour, min, sec, nsec );
    } // uTime::uTime

    uTime( int month, int day, int hour, int min, int sec, long int nsec ) {
	uCreateTime( 1970, month, day, hour, min, sec, nsec );
    } // uTime::uTime

    uTime( int year, int month, int day, int hour, int min, int sec, long int nsec ) {
	uCreateTime( year, month, day, hour, min, sec, nsec );
    } // uTime::uTime

    uTime( const timeval &t ) {
	tv = (long long int)t.tv_sec * TIMEGRAN + t.tv_usec * 1000;
    } // uTime::uTime

    uTime( const timespec &t ) {
	tv = (long long int)t.tv_sec * TIMEGRAN + t.tv_nsec;
    } // uTime::uTime

    uTime &operator=( const timeval &t ) {
	tv = (long long int)t.tv_sec * TIMEGRAN + t.tv_usec * 1000;
	return *this;
    } // uTime::operator=

    uTime &operator=( const timespec &t ) {
	tv = (long long int)t.tv_sec * TIMEGRAN + t.tv_nsec;
	return *this;
    } // uTime::operator=

    operator timeval() const {
	timeval dummy;
	dummy.tv_sec = tv / TIMEGRAN;			// seconds
	dummy.tv_usec = tv % TIMEGRAN / ( TIMEGRAN / 1000000L ); // microseconds
	return dummy;
    } // uTime::operator timeval

    operator timespec() const {
	timespec dummy;
	dummy.tv_sec = tv / TIMEGRAN;			// seconds
	dummy.tv_nsec = tv % TIMEGRAN;			// nanoseconds
	return dummy;
    } // uTime::operator timespec

    long long int nanoseconds() const {
	return tv;
    } // uTime::longlongint

    uTime &operator-=( uDuration op ) {
	*this = *this - op;
	return *this;
    } // uTime::operator-=

    uTime &operator+=( uDuration op ) {
	*this = *this + op;
	return *this;
    } //  uTime::operator+=
}; // uTime


inline uTime operator+( uTime op1, uDuration op2 ) {
    uTime ans;
    ans.tv = op1.tv + op2.tv;
    return ans;
} // operator+

inline uTime operator+( uDuration op1, uTime op2 ) {
    return op2 + op1;
} // operator+

inline uDuration operator-( uTime op1, uTime op2 ) {
    uDuration ans;
    ans.tv = op1.tv - op2.tv;
    return ans;
} // operator-

inline uTime operator-( uTime op1, uDuration op2 ) {
    uTime ans;
    ans.tv = op1.tv - op2.tv;
    return ans;
} // operator-

inline bool operator==( uTime op1, uTime op2 ) {
    return op1.tv == op2.tv;
} // operator==

inline bool operator!=( uTime op1, uTime op2 ) {
    return op1.tv != op2.tv;
} // operator!=

inline bool operator>( uTime op1, uTime op2 ) {
    return op1.tv > op2.tv;
} // operator>

inline bool operator<( uTime op1, uTime op2 ) {
    return op1.tv < op2.tv;
} // operator<

inline bool operator>=( uTime op1, uTime op2 ) { 
    return op1.tv >= op2.tv;
} // operator>=

inline bool operator<=( uTime op1, uTime op2 ) {
    return op1.tv <= op2.tv;
} // operator<=


//######################### uClock #########################


class uClock {
    uTime offset;					// for virtual clock: contains offset from real-time
    int clocktype;					// implementation only -1 (virtual), CLOCK_REALTIME
  public:
    uClock() {
	clocktype = CLOCK_REALTIME;
    } // uClock::uClock

    uClock( uTime adj ) {
	resetClock( adj );
    } // uClock::uClock

    void resetClock() {
	clocktype = CLOCK_REALTIME;
    } // uClock::resetClock

    void resetClock( uTime adj );

    uTime getTime();
    void getTime( int &year, int &month, int &day, int &hour, int &minutes, int &seconds, long int &nsec );

    static void convertTime( uTime time, int &year, int &month, int &day, int &hour, int &minutes, int &seconds, long int &nsec );
}; // uClock


#endif // __U_CALENDAR_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
