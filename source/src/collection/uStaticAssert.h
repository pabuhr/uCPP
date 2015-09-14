// (C) Copyright John Maddock 2000.
// Permission to copy, use, modify, sell and distribute this software is granted provided this copyright notice appears
// in all copies. This software is provided "as is" without express or implied warranty, and with no claim as to its
// suitability for any purpose.

// See http://www.boost.org/libs/static_assert for documentation.


#ifndef __U_STATIC_ASSERT_H__
#define __U_STATIC_ASSERT_H__

namespace _static_assert_ {
    template<bool> struct STATIC_ASSERTION_FAILURE;
    template<> struct STATIC_ASSERTION_FAILURE<true> {};
    template<int x> struct static_assert_test {};    
} // _static_assert_

#define _CPP_CONCAT_( X, Y ) _CPP_DO_CONCAT_( X, Y )
#define _CPP_DO_CONCAT_( X, Y ) X##Y

#define _STATIC_ASSERT_( B ) typedef ::_static_assert_::static_assert_test< sizeof(::_static_assert_::STATIC_ASSERTION_FAILURE< (bool)( B ) >) > \
	_CPP_CONCAT_( _static_assert_typedef_, __LINE__ ) __attribute__(( unused ))

#endif // __U_STATIC_ASSERT_H__

// Local Variables: //
// compile-command: "make install" //
// End: //
