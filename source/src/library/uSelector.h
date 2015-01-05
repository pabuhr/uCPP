//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Richard C. Bilson 2007
// 
// Selector.h -- 
// 
// Author           : Richard C. Bilson
// Created On       : Mon Jul 16 07:45:28 2007
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Dec 14 23:36:57 2007
// Update Count     : 5
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

#ifndef __U_SELECTOR_H__
#define __U_SELECTOR_H__


struct uBaseAction {
    virtual ~uBaseAction() {}
    virtual void operator()() const = 0;
}; // uBaseAction


namespace UPP {
    template< typename Condition, typename Selector >
    class SelectBuilder {
	typedef SelectBuilder< Condition, Selector > ThisSelectBuilder;
	template< typename C, typename S, typename ActionType > friend SelectBuilder< OrCondition, UnarySelector< S, const uBaseAction* > > orselect( const SelectBuilder< C, S > &q, const ActionType &act );
	template< typename C, typename S, typename ActionType > friend SelectBuilder< OrCondition, UnarySelector< S, const uBaseAction* > > orselect( bool guard, const SelectBuilder< C, S > &q, const ActionType &act );
	template< typename C, typename S > friend SelectBuilder< OrCondition, UnarySelector< S, const uBaseAction* > > orselect( bool guard, const SelectBuilder< C, S > &q );
	template< typename C, typename S > friend SelectBuilder< OrCondition, S > orselect( const SelectBuilder< C, S > &q );
	template< typename C, typename S, typename ActionType > friend SelectBuilder< AndCondition, UnarySelector< S, const uBaseAction* > > andselect( const SelectBuilder< C, S > &q, const ActionType &act );
	template< typename C, typename S, typename ActionType > friend SelectBuilder< AndCondition, UnarySelector< S, const uBaseAction* > > andselect( bool guard, const SelectBuilder< C, S > &q, const ActionType &act );
	template< typename C, typename S > friend SelectBuilder< AndCondition, UnarySelector< S, const uBaseAction* > > andselect( bool guard, const SelectBuilder< C, S > &q );
	template< typename C, typename S > friend SelectBuilder< AndCondition, S > andselect( const SelectBuilder< C, S > &q );
	template< typename C, typename S > friend SelectBuilder< C, S > when( bool guard, const SelectBuilder< C, S > &q );

	Selector selector;
	bool hasElse;
	const uBaseAction *elseAction;
	bool hasTimeout;
	uTime time;
	const uBaseAction *timeoutAction;
	mutable bool armed;

      public:
	SelectBuilder( const Selector &s ) : selector( s ), hasElse( false ), elseAction( NULL ), hasTimeout( false ), timeoutAction( false ), armed( true ) {}

	SelectBuilder( const SelectBuilder &other ) : selector( other.selector ), hasElse( other.hasElse ), elseAction( other.elseAction ), hasTimeout( other.hasTimeout ), time( other.time ), timeoutAction( other.timeoutAction ), armed( true ) {
	    other.armed = false;
	} // SelectBuilder::SelectBuilder

	SelectBuilder &operator=( const SelectBuilder &other ) {
	    if ( this == &other ) return other;
	    selector = other.selector;
	    hasElse = other.hasElse;
	    elseAction = other.elseAction;
	    hasTimeout = other.hasTimeout;
	    time = other.time;
	    timeoutAction = other.timeoutAction;
	    armed = true;
	    other.armed = false;
	} // SelectBuilder::operator=

	~SelectBuilder() {
	    if ( armed && ! std::uncaught_exception() ) wait();
	} // SelectBuilder::~SelectBuilder

	template< typename OtherCondition, typename OtherSelector >
	SelectBuilder< Condition, BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * > > operator()( bool guard, const SelectBuilder< OtherCondition, OtherSelector > &q, const uBaseAction &act ) {
	    q.armed = false;
	    armed = false;
	    return SelectBuilder< Condition, BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * > >( BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * >( selector, q.selector.addAction( &act ).addGuard( guard ) ) );
	} // operator()

	template< typename OtherCondition, typename OtherSelector >
	SelectBuilder< Condition, BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * > > operator()( const SelectBuilder< OtherCondition, OtherSelector > &q, const uBaseAction &act ) {
	    q.armed = false;
	    armed = false;
	    return SelectBuilder< Condition, BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * > >( BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * >( selector, q.selector.addAction( &act ) ) );
	} // operator()

	template< typename OtherCondition, typename OtherSelector >
	SelectBuilder< Condition, BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * > > operator()( bool guard, const SelectBuilder< OtherCondition, OtherSelector > &q ) {
	    q.armed = false;
	    armed = false;
	    return SelectBuilder< Condition, BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * > >( BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * >( selector, q.selector.addGuard( guard ) ) );
	} // operator()

	template< typename OtherCondition, typename OtherSelector >
	SelectBuilder< Condition, BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * > > operator()( const SelectBuilder< OtherCondition, OtherSelector > &q ) {
	    q.armed = false;
	    armed = false;
	    return SelectBuilder< Condition, BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * > >( BinarySelector< Condition, Selector, OtherSelector, const uBaseAction * >( selector, q.selector ) );
	} // operator()

	template< typename Next >
	SelectBuilder< Condition, BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * > > operator()( const Next &q, const uBaseAction &act ) {
	    armed = false;
	    return SelectBuilder< Condition, BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * > >( BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * >( selector, UnarySelector< Next, const uBaseAction * >( true, q, &act ) ) );
	} // operator()

	template< typename Next >
	SelectBuilder< Condition, BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * > > operator()( bool guard, const Next &q, const uBaseAction &act ) {
	    armed = false;
	    return SelectBuilder< Condition, BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * > >( BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * >( selector, UnarySelector< Next, const uBaseAction * >( guard, q, &act ) ) );
	} // operator()

	template< typename Next >
	SelectBuilder< Condition, BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * > > operator()( const Next &q ) {
	    armed = false;
	    return SelectBuilder< Condition, BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * > >( BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * >( selector, UnarySelector< Next, const uBaseAction * >( true, q ) ) );
	} // operator()

	template< typename Next >
	SelectBuilder< Condition, BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * > > operator()( bool guard, const Next &q ) {
	    armed = false;
	    return SelectBuilder< Condition, BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * > >( BinarySelector< Condition, Selector, UnarySelector< Next, const uBaseAction * >, const uBaseAction * >( selector, UnarySelector< Next, const uBaseAction * >( guard, q ) ) );
	} // operator()

	ThisSelectBuilder orelse( bool guard = true ) {
	    if ( hasElse ) uAbort( "duplicate orelse clause in select" );
	    if ( guard && hasTimeout ) uAbort( "both else and timeout clause in select" );
	    ThisSelectBuilder copy( *this );
	    copy.hasElse = guard;
	    copy.elseAction = NULL;
	    return copy;
	} // orelse

	ThisSelectBuilder orelse( const uBaseAction &act ) {
	    if ( hasElse ) uAbort( "duplicate orelse clause in select" );
	    if ( hasTimeout ) uAbort( "both else and timeout clause in select" );
	    ThisSelectBuilder copy( *this );
	    copy.hasElse = true;
	    copy.elseAction = &act;
	    return copy;
	} // orelse

	ThisSelectBuilder orelse( bool guard, const uBaseAction &act ) {
	    if ( hasElse ) uAbort( "duplicate orelse clause in select" );
	    if ( guard && hasTimeout ) uAbort( "both else and timeout clause in select" );
	    ThisSelectBuilder copy( *this );
	    copy.hasElse = guard;
	    copy.elseAction = &act;
	    return copy;
	} // orelse

	ThisSelectBuilder timeout( uTime time ) {
	    if ( hasTimeout ) uAbort( "duplicate timeout clause in select" );
	    if ( hasElse ) uAbort( "both else and timeout clause in select" );
	    ThisSelectBuilder copy( *this );
	    copy.hasTimeout = true;
	    copy.time = time;
	    copy.timeoutAction = NULL;
	    return copy;
	} // orelse

	ThisSelectBuilder timeout( bool guard, uTime time ) {
	    if ( hasTimeout ) uAbort( "duplicate timeout clause in select" );
	    if ( guard && hasElse ) uAbort( "both else and timeout clause in select" );
	    ThisSelectBuilder copy( *this );
	    copy.hasTimeout = guard;
	    copy.time = time;
	    copy.timeoutAction = NULL;
	    return copy;
	} // orelse

	ThisSelectBuilder timeout( uTime time, const uBaseAction &act ) {
	    if ( hasTimeout ) uAbort( "duplicate timeout clause in select" );
	    if ( hasElse ) uAbort( "both else and timeout clause in select" );
	    ThisSelectBuilder copy( *this );
	    copy.hasTimeout = true;
	    copy.time = time;
	    copy.timeoutAction = &act;
	    return copy;
	} // orelse

	ThisSelectBuilder timeout( bool guard, uTime time, const uBaseAction &act ) {
	    if ( hasTimeout ) uAbort( "duplicate timeout clause in select" );
	    if ( guard && hasElse ) uAbort( "both else and timeout clause in select" );
	    ThisSelectBuilder copy( *this );
	    copy.hasTimeout = guard;
	    copy.time = time;
	    copy.timeoutAction = &act;
	    return copy;
	} // orelse

	ThisSelectBuilder timeout( uDuration duration ) {
	    return timeout( uThisProcessor().getClock().getTime() + duration );
	} // orelse

	ThisSelectBuilder timeout( bool guard, uDuration duration ) {
	    return timeout( guard, uThisProcessor().getClock().getTime() + duration );
	} // orelse

	ThisSelectBuilder timeout( uDuration duration, const uBaseAction &act ) {
	    return timeout( uThisProcessor().getClock().getTime() + duration, act );
	} // orelse

	ThisSelectBuilder timeout( bool guard, uDuration duration, const uBaseAction &act ) {
	    return timeout( guard, uThisProcessor().getClock().getTime() + duration, act );
	} // orelse

	void wait() {
	    const uBaseAction *todo;
	    bool hasAction_, isAvailable;
	    armed = false;
	    if ( hasElse ) {
		if ( ! tryNextAction( selector, hasAction_, todo ) ) {
		    if ( elseAction ) (*elseAction)();
		} else {
		    do {
			if ( hasAction_ && todo != NULL ) (*todo)();
		    } while( tryNextAction( selector, hasAction_, todo ) && hasAction_ );
		} // if
	    } else {
		do {
		    if ( hasTimeout ) {
			isAvailable = nextAction( selector, hasAction_, todo, time );
			if ( !isAvailable && !hasAction_ ) {
			    if ( timeoutAction ) (*timeoutAction)();
			    break;
			} // if
		    } else {
			isAvailable = nextAction( selector, hasAction_, todo );
		    } // if
		    //cerr << "wait: nextAction returned isAvailable " << isAvailable << " hasAction_ " << hasAction_ << " todo " << todo << endl;
		    //if ( hasAction_ && todo != NULL ) cerr << "wait got action " << todo << endl;
		    if ( hasAction_ && todo != NULL ) (*todo)();
		} while ( ! isAvailable || hasAction_ );
	    } // if
	}
    }; // SelectBuilder

    // orselect

    template< typename Selectee, typename ActionType >
    SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction* > > orselect( const Selectee &q, const ActionType &act ) {
	return SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction* > >( UnarySelector< Selectee, const uBaseAction* >( true, q, &act ) );
    } // orselect

    template< typename Selectee, typename ActionType >
    SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction* > > orselect( bool guard, const Selectee &q, const ActionType &act ) {
	return SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction* > >( UnarySelector< Selectee, const uBaseAction* >( guard, q, &act ) );
    } // orselect

    template< typename Selectee >
    SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction* > > orselect( const Selectee &q ) {
	return SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction* > >( UnarySelector< Selectee, const uBaseAction* >( q ) );
    } // orselect

    template< typename Selectee >
    SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction* > > orselect( bool guard, const Selectee &q ) {
	return SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction* > >( UnarySelector< Selectee, const uBaseAction* >( guard, q ) );
    } // orselect

    template< typename Condition, typename Selector, typename ActionType >
    SelectBuilder< OrCondition, UnarySelector< Selector, const uBaseAction* > > orselect( const SelectBuilder< Condition, Selector > &q, const ActionType &act ) {
	q.armed = false;
	return SelectBuilder< OrCondition, UnarySelector< Selector, const uBaseAction* > >( UnarySelector< Selector, const uBaseAction* >( true, q.selector, &act ) );
    } // orselect

    template< typename Condition, typename Selector, typename ActionType >
    SelectBuilder< OrCondition, UnarySelector< Selector, const uBaseAction* > > orselect( bool guard, const SelectBuilder< Condition, Selector > &q, const ActionType &act ) {
	q.armed = false;
	return SelectBuilder< OrCondition, UnarySelector< Selector, const uBaseAction* > >( UnarySelector< Selector, const uBaseAction* >( guard, q.selector, &act ) );
    } // orselect

    template< typename Condition, typename Selector >
    SelectBuilder< OrCondition, UnarySelector< Selector, const uBaseAction* > > orselect( bool guard, const SelectBuilder< Condition, Selector > &q ) {
	q.armed = false;
	return SelectBuilder< OrCondition, UnarySelector< Selector, const uBaseAction* > >( UnarySelector< Selector, const uBaseAction* >( guard, q.selector ) );
    } // orselect

    // optimization: if not adding an action or guard, no need to wrap selector
    template< typename Condition, typename Selector >
    SelectBuilder< OrCondition, Selector > orselect( const SelectBuilder< Condition, Selector > &q ) {
	q.armed = false;
	return SelectBuilder< OrCondition, Selector >( q.selector );
    } // orselect

    // andselect

    template< typename Selectee, typename ActionType >
    SelectBuilder< AndCondition, UnarySelector< Selectee, const uBaseAction* > > andselect( const Selectee &q, const ActionType &act ) {
	return SelectBuilder< AndCondition, UnarySelector< Selectee, const uBaseAction* > >( UnarySelector< Selectee, const uBaseAction* >( true, q, &act ) );
    } // andselect

    template< typename Selectee, typename ActionType >
    SelectBuilder< AndCondition, UnarySelector< Selectee, const uBaseAction* > > andselect( bool guard, const Selectee &q, const ActionType &act ) {
	return SelectBuilder< AndCondition, UnarySelector< Selectee, const uBaseAction* > >( UnarySelector< Selectee, const uBaseAction* >( guard, q, &act ) );
    } // andselect

    template< typename Selectee >
    SelectBuilder< AndCondition, UnarySelector< Selectee, const uBaseAction* > > andselect( const Selectee &q ) {
	return SelectBuilder< AndCondition, UnarySelector< Selectee, const uBaseAction* > >( UnarySelector< Selectee, const uBaseAction* >( true, q ) );
    } // andselect

    template< typename Selectee >
    SelectBuilder< AndCondition, UnarySelector< Selectee, const uBaseAction* > > andselect( bool guard, const Selectee &q ) {
	return SelectBuilder< AndCondition, UnarySelector< Selectee, const uBaseAction* > >( UnarySelector< Selectee, const uBaseAction* >( guard, q ) );
    } // andselect

    template< typename Condition, typename Selector, typename ActionType >
    SelectBuilder< AndCondition, UnarySelector< Selector, const uBaseAction* > > andselect( const SelectBuilder< Condition, Selector > &q, const ActionType &act ) {
	q.armed = false;
	return SelectBuilder< AndCondition, UnarySelector< Selector, const uBaseAction* > >( UnarySelector< Selector, const uBaseAction* >( true, q.selector, &act ) );
    } // andselect

    template< typename Condition, typename Selector, typename ActionType >
    SelectBuilder< AndCondition, UnarySelector< Selector, const uBaseAction* > > andselect( bool guard, const SelectBuilder< Condition, Selector > &q, const ActionType &act ) {
	q.armed = false;
	return SelectBuilder< AndCondition, UnarySelector< Selector, const uBaseAction* > >( UnarySelector< Selector, const uBaseAction* >( guard, q.selector, &act ) );
    } // andselect

    template< typename Condition, typename Selector >
    SelectBuilder< AndCondition, UnarySelector< Selector, const uBaseAction* > > andselect( bool guard, const SelectBuilder< Condition, Selector > &q ) {
	q.armed = false;
	return SelectBuilder< AndCondition, UnarySelector< Selector, const uBaseAction* > >( UnarySelector< Selector, const uBaseAction* >( guard, q.selector ) );
    } // andselect

    // optimization: if not adding an action or guard, no need to wrap selector
    template< typename Condition, typename Selector >
    SelectBuilder< AndCondition, Selector > andselect( const SelectBuilder< Condition, Selector > &q ) {
	q.armed = false;
	return SelectBuilder< AndCondition, Selector >( q.selector );
    } // andselect

    // when

    template< typename Condition, typename Selector >
    SelectBuilder< Condition, Selector > when( bool guard, const SelectBuilder< Condition, Selector > &q ) {
	q.armed = false;
	return SelectBuilder< Condition, Selector >( q.selector.addGuard( guard ) );
    } // when

    template< typename Selectee >
    SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction * > > when( bool guard, const Selectee &selectee ) {
	return SelectBuilder< OrCondition, UnarySelector< Selectee, const uBaseAction * > >( UnarySelector< Selectee, const uBaseAction * >( guard, selectee ) );
    } // when
} // UPP

using UPP::orselect;
using UPP::andselect;
using UPP::when;
 

#endif // __U_SELECTOR_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
