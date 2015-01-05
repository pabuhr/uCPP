//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Richard C. Bilson 2007
// 
// uBaseSelector.h -- 
// 
// Author           : Richard C. Bilson
// Created On       : Sat Jul 14 07:25:52 2007
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed May 14 15:23:54 2014
// Update Count     : 154
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


#ifndef __U_BASESELECTOR_H__
#define __U_BASESELECTOR_H__

//#include <iostream>

namespace UPP {
    // The complexity of this algorithm stems from the lack of lambda routines in original C++. As a result, the
    // evaluation of the _Select expression must keep returning to the root of the tree so the code in the select
    // clauses can be executed, and then the tree must be traversed again to determine when the expression is complete.
    // Passing the select clause as lambdas would allow a single pass through the tree, but the lambdas may be more
    // expensive to execute. To mitigate having to restart the tree walk, nodes in the tree that have triggered
    // execution of a select clause are pruned so the tree walk is shortened for each selected clause.

    // The evaluation tree is composed of unary and binary nodes, and futures. A unary node represents a future or
    // binary node, with an optional action from the _Select clause. A unary node represents a binary node for this
    // case:
    //
    //    _Select( f1 || f2 ) A;
    //
    // because the expression 
    // A binary node represents a logical operation (|| or &&) relating unary or future nodes.

    //  U(A)  U(A)        B(no A)
    //  |     |         /   \x
    //  F     B        U|B U|B

    // Tree nodes are built and chained together on the stack of the thread executing the _Select statement, so there is
    // no dynamic allocation. The code is generalized to allow potential use in other situations.
    
    struct BaseFutureDL : public uSeqable {
	virtual void signal() = 0;
	virtual ~BaseFutureDL() {}
    }; // BaseFutureDL

    struct SelectorClient {
	uSemaphore sem;					// selection client waits if no future available
	SelectorClient() : sem( 0 ) {};
    }; // SelectorClient

    struct SelectorDL : public BaseFutureDL {
	SelectorClient *client;				// client data for server

	virtual ~SelectorDL() {}

	virtual void signal() {
	    client->sem.V();
	} // SelectorDL::signal
    }; // SelectorDL

    class OrCondition {
      public:
	static bool sat( bool left, bool right ) {
	    //std::osacquire( std::cerr ) << "OrCondition::sat left " << left << " right " << right << " result " << (left || right) << std::endl;
	    return left || right;
	} // OrCondition::operator()

	static bool shortcircuit( bool left ) {
	    //std::osacquire( std::cerr ) << "OrCondition::shortcircuit left " << left << " result " << left << std::endl;
	    return left;
	} // OrCondition::shortcircuit
    }; // OrCondition

    class AndCondition {
      public:
	static bool sat( bool left, bool right ) {
	    //std::osacquire( std::cerr ) << "AndCondition::sat left " << left << " right " << right << std::endl;
	    return left && right;
	} // AndCondition::operator()

	static bool shortcircuit( bool left ) {
	    //std::osacquire( std::cerr ) << "AndCondition::shortcircuit left " << left << std::endl;
	    return false;
	} // AndCondition::shortcircuit
    }; // AndCondition

    enum SelectorStatus { NAvail = false, Avail = true, GuardFalse };

    template< typename Selectee > struct Helper;
    template< typename Condition, typename Left, typename Right, typename ActionType > class BinarySelector;

    // UnarySelector serves three purposes:
    // - attaches an action to a selectable
    // - attaches a guard to a selectable
    // - provides a copyable wrapper for a non-copyable selectable (e.g., Future_ESM)
    template< typename Selectee, typename ActionType > class UnarySelector {
	typedef UnarySelector< Selectee, ActionType > ThisSelector;
	template< typename S > friend struct Helper;
	SelectorDL baseFuture;				// 
	Selectee &selectee;				// future in _Select clause
	ActionType action;				// (int) key to locate _Select action in switch (conceptually pointer to lambda)
	bool guard;					// value of _When guard at start of _Select; no guard => true, i.e., _When( true )
	bool doRemove;
	bool hasAction;					// futures in an expression, e.g., f1 || f2, have no action
      public:
	UnarySelector( const Selectee &q, ActionType act ) : selectee( const_cast< Selectee& >( q ) ), action( act ), guard( true ), doRemove( false ), hasAction( true ) {
	} // UnarySelector::UnarySelector

	UnarySelector( bool guard_, const Selectee &q, ActionType act ) : selectee( const_cast< Selectee& >( q ) ), action( act ), guard( guard_ ), doRemove( false ), hasAction( true ) {
	} // UnarySelector::UnarySelector

	UnarySelector( const Selectee &q ) : selectee( const_cast< Selectee& >( q ) ), guard( true ), doRemove( false ), hasAction( false ) {
	} // UnarySelector::UnarySelector

	UnarySelector( bool guard_, const Selectee &q ) : selectee( const_cast< Selectee& >( q ) ), guard( guard_ ), doRemove( false ), hasAction( false ) {
	} // UnarySelector::UnarySelector

//	ThisSelector addAction( const ActionType &act ) const {
//	    ThisSelector ret( *this );
//	    ret.action = act;
//	    return ret;
//	} // UnarySelector::addAction

//	ThisSelector addGuard( bool guard ) const {
//	    ThisSelector ret( *this );
//	    ret.guard = guard;
//	    return ret;
//	} // UnarySelector::addGuard

	SelectorStatus addAccept( SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
	    //std::osacquire( std::cerr ) << "UnarySelector::addAccept " << this << " guard " << guard << std::endl;
	    if ( ! guard ) return GuardFalse;
	    SelectorStatus child = Helper< Selectee >::addAccept( selectee, acceptState, hasAction_, action_ );
	    if ( child == Avail && ! hasAction_ && hasAction ) {
		hasAction = false;
		hasAction_ = true;
		//std::osacquire( std::cerr ) << "UnarySelector::addAccept assigning action " << action << " hasAction_ " << hasAction_  << std::endl;
		action_ = action;
	    } // if
	    doRemove = ( child == NAvail );
	    ////std::osacquire( std::cerr ) << "UnarySelector::addAccept " << this << " returns " << child << std::endl;
	    return child;
	} // UnarySelector::addAccept

	void removeAccept( SelectorDL *acceptState ) {
	    //std::osacquire( std::cerr ) << "UnarySelector::removeAccept enter " << this << " acceptState " << acceptState << std::endl;
	    if ( doRemove ) {
		selectee.removeAccept( acceptState );
		doRemove = false;
	    } // if
	} // UnarySelector::removeAccept

	SelectorStatus available( bool &hasAction_, ActionType &action_ ) {
	    //std::osacquire( std::cerr ) << "UnarySelector::available enter " << std::endl;
	    if ( ! guard ) return GuardFalse;
	    SelectorStatus child = Helper< Selectee >::available( selectee, hasAction_, action_ );
	    if ( child == Avail && ! hasAction_ && hasAction ) { // childaction
		hasAction_ = true;
		//std::osacquire( std::cerr ) << "UnarySelector::available assigning action " << action << std::endl;
		action_ = action;
		hasAction = false;
	    } // if
	    return child;
	} // UnarySelector::available

//	template< typename Other > BinarySelector< OrCondition, ThisSelector, UnarySelector< Other, int >, int > operator||( const Other &s2 ) {
//	    return BinarySelector< OrCondition, ThisSelector, UnarySelector< Other, int >, int >( *this, UnarySelector< Other, int >( s2 ) );
//	} // UnarySelector::operator||

//	template< typename Other >
//	BinarySelector< AndCondition, ThisSelector, UnarySelector< Other, int >, int > operator&&( const Other &s2 ) {
//	    return BinarySelector< AndCondition, ThisSelector, UnarySelector< Other, int >, int >( *this, UnarySelector< Other, int >( s2 ) );
//	} // UnarySelector::operator&&
    }; // UnarySelector

    // Left and Right must be copyable and selectable types 
    template< typename Condition, typename Left, typename Right, typename ActionType > class BinarySelector {
	template< typename C, typename L, typename R, typename A > friend BinarySelector< C, L, R, A > &when( bool cond, BinarySelector< C, L, R, A > &selectee );
	typedef BinarySelector< Condition, Left, Right, ActionType > ThisSelector;
	template< typename S > friend struct Helper;
	SelectorDL baseFuture;
	Left left;					// left branch (unary or binary selector)
	Right right;					// right branch (unary or binary selector)
//	ActionType action;				// key used to locate _Select action in switch
	bool guard;					// value of _When guard at start of _Select; no guard => true, i.e., _When( true )
	bool removeLeft, removeRight;
//	bool hasAction;
      public:
	// BinarySelector does not have an action because the actions are associated with the UnarySelectors of leafs.
//	BinarySelector( const Left &left_, const Right &right_, ActionType act ) : left( left_ ), right( right_ ), action( act ), guard( true ), removeLeft( false ), removeRight( false ), hasAction( true ) {
//	} // BinarySelector::BinarySelector

//	BinarySelector( bool guard_, const Left &left_, const Right &right_, ActionType act ) : left( left_ ), right( right_ ), action( act ), guard( guard_ ), removeLeft( false ), removeRight( false ), hasAction( true ) {
//	} // BinarySelector::BinarySelector

	BinarySelector( const Left &left_, const Right &right_ ) : left( left_ ), right( right_ ), guard( true ), removeLeft( false ), removeRight( false )/*, hasAction( false )*/ {
	} // BinarySelector::BinarySelector

	BinarySelector( bool guard_, const Left &left_, const Right &right_ ) : left( left_ ), right( right_ ), guard( guard_ ), removeLeft( false ), removeRight( false )/*, hasAction( false )*/ {
	} // BinarySelector::BinarySelector

//	ThisSelector addAction( ActionType act ) const {
//	    ThisSelector ret( *this );
//	    ret.action = act;
//	    return ret;
//	} // BinarySelector::addAction

//	ThisSelector addGuard( bool guard ) const {
//	    ThisSelector ret( *this );
//	    ret.guard = guard;
//	    return ret;
//	} // BinarySelector::addGuard

	// Called on the 
	SelectorStatus addAccept( SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
	    //std::osacquire( std::cerr ) << "BinarySelector::addAccept " << this << " guard " << guard << std::endl;
	    if ( ! guard ) return GuardFalse;
	    SelectorStatus sat;
	    SelectorStatus leftstat = Helper< Left >::addAccept( left, acceptState, hasAction_, action_ );
	    //std::osacquire( std::cerr ) << "BinarySelector::addAccept " << this << " left child " << &left << " leftstat " << leftstat << " acceptState " << acceptState << " hasAction_ " << hasAction_ << std::endl;
	    removeLeft = ( leftstat == NAvail );
	    if ( Condition::shortcircuit( leftstat == Avail ) ) {
		sat = Avail;
	    } else {
		baseFuture.client = acceptState->client;
		SelectorStatus rightstat = Helper< Right >::addAccept( right, &baseFuture, hasAction_, action_ );
		//std::osacquire( std::cerr ) << "BinarySelector::addAccept " << this << " right child " << &right << " rightstat " << rightstat << " acceptState " << acceptState << " hasAction_ " << hasAction_ << std::endl;
		removeRight = ( rightstat == NAvail );
		if ( leftstat == GuardFalse && rightstat == GuardFalse ) {
		    sat = GuardFalse;
		} else if ( leftstat == GuardFalse ) {
		    sat = rightstat;
		} else if ( rightstat == GuardFalse ) {
		    sat = leftstat;
		} else {
		    sat = (SelectorStatus)Condition::sat( (bool)leftstat, (bool)rightstat );
		} // if
	    } // if
	    if ( sat == Avail && ! hasAction_ /*&& hasAction*/ ) {
//		hasAction = false;
		hasAction_ = true;
//		//std::osacquire( std::cerr ) << "BinarySelector::addAccept assigning action " << action << std::endl;
//		action_ = action;
	    } // if
	    if ( sat != NAvail ) {
		removeAccept( acceptState );
	    } // if
	    //std::osacquire( std::cerr ) << "BinarySelector::addAccept " << this << " returns " << sat << std::endl;
	    return sat;
	} // BinarySelector::addAccept

	void removeAccept( SelectorDL *acceptState ) {
	    //std::osacquire( std::cerr ) << "BinarySelector::removeAccept enter " << this << " acceptState " << acceptState << std::endl;
	    if ( ! guard ) return;
	    if ( removeLeft ) {
		left.removeAccept( acceptState );
		removeLeft = false;
	    } // if
	    if ( removeRight ) {
		right.removeAccept( &baseFuture );
		removeRight = false;
	    } // if
	} // BinarySelector::removeAccept

	SelectorStatus available( bool &hasAction_, ActionType &action_ ) {
	    //std::osacquire( std::cerr ) << "BinarySelector::available " << this << "enter" << std::endl;
	    if ( ! guard ) return GuardFalse;
	    // it is necessary here to inquire specifically about the guard of the left child, rather than simply
	    // recursing, because the recursion could return true either because the guard is false or because the guard
	    // is true and the sub-expression is satisfied. It is important to be able to distinguish these two cases to
	    // decide when to short-circuit.
	    SelectorStatus sat;
	    SelectorStatus leftstat = Helper< Left >::available( left, hasAction_, action_ );
	    //std::osacquire( std::cerr ) << "BinarySelector::available " << this << " left child " << &left << " leftstat " << leftstat << " hasAction_ " << hasAction_ << std::endl;
	    if ( Condition::shortcircuit( leftstat == Avail ) ) {
		sat = Avail;
	    } else {
		SelectorStatus rightstat = Helper< Right >::available( right, hasAction_, action_ );
		//std::osacquire( std::cerr ) << "BinarySelector::available " << this << " rightstat " << rightstat << " hasAction_ " << hasAction_ << std::endl;
		if ( leftstat == GuardFalse && rightstat == GuardFalse ) {
		    sat = GuardFalse;
		} else if ( leftstat == GuardFalse ) {
		    sat = rightstat;
		} else if ( rightstat == GuardFalse ) {
		    sat = leftstat;
		} else {
		    sat = (SelectorStatus)Condition::sat( (bool)leftstat, (bool)rightstat );
		} // if
	    } // if
	    if ( sat == Avail && ! hasAction_ /*&& hasAction*/ ) {
//		hasAction = false;
		hasAction_ = true;
//		//std::osacquire( std::cerr ) << "BinarySelector::available assigning action " << action << std::endl;
//		action_ = action;
	    } // if
	    //std::osacquire( std::cerr ) << "BinarySelector::available " << this << " returns " << sat << std::endl;
	    return sat;
	} // BinarySelector::available

	template< typename Other >
	BinarySelector< OrCondition, ThisSelector, UnarySelector< Other, int >, int > operator||( const Other &s2 ) {
	    return BinarySelector< OrCondition, ThisSelector, UnarySelector< Other, int >, int >( *this, UnarySelector< Other, int >( s2 ) );
	} // BinarySelector::operator||

	template< typename Other >
	BinarySelector< AndCondition, ThisSelector, UnarySelector< Other, int >, int > operator&&( const Other &s2 ) {
	    return BinarySelector< AndCondition, ThisSelector, UnarySelector< Other, int >, int >( *this, UnarySelector< Other, int >( s2 ) );
	} // BinarySelector::operator&&
    }; // BinarySelector

    template< typename Selectee > struct Helper {
	template< typename ActionType > static SelectorStatus addAccept( Selectee &s, SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
	    return (SelectorStatus)s.addAccept( acceptState );
	} // Helper::addAccept

	template< typename ActionType > static SelectorStatus available( Selectee &s, bool &hasAction_, ActionType &action_ ) {
	    return (SelectorStatus)s.available();
	} // Helper::available
    }; // Helper

    template< typename Condition, typename Left, typename Right, typename ActionType >
    struct Helper< BinarySelector< Condition, Left, Right, ActionType > > {
	static SelectorStatus addAccept( BinarySelector< Condition, Left, Right, ActionType > &s, SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
	    return s.addAccept( acceptState, hasAction_, action_ );
	} // Helper::addAccept

	template< typename OtherAction > static SelectorStatus addAccept( BinarySelector< Condition, Left, Right, ActionType > &s, SelectorDL *acceptState, bool &hasAction_, OtherAction &action_ ) {
	    bool ha;
	    ActionType act;
	    return s.addAccept( acceptState, ha, act );
	} // Helper::addAccept

	static SelectorStatus available( BinarySelector< Condition, Left, Right, ActionType > &s, bool &hasAction_, ActionType &action_ ) {
	    return s.available( hasAction_, action_ );
	} // available

	template< typename OtherAction > static SelectorStatus available( BinarySelector< Condition, Left, Right, ActionType > &s, bool &hasAction_, OtherAction &action_ ) {
	    bool ha;
	    ActionType act;
	    return s.available( ha, act );
	} // Helper::available
    }; // Helper

    template< typename Selectee, typename ActionType >
    struct Helper< UnarySelector< Selectee, ActionType > > {
	static SelectorStatus addAccept( UnarySelector< Selectee, ActionType > &s, SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
	    return s.addAccept( acceptState, hasAction_, action_ );
	} // Helper::addAccept

	template< typename OtherAction > static SelectorStatus addAccept( UnarySelector< Selectee, ActionType > &s, SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
	    bool ha;
	    ActionType act;
	    return s.addAccept( acceptState, ha, act );
	} // Helper::addAccept

	static SelectorStatus available( UnarySelector< Selectee, ActionType > &s, bool &hasAction_, ActionType &action_ ) {
	    return s.available( hasAction_, action_ );
	} // Helper::available

	template< typename OtherAction > static SelectorStatus available( UnarySelector< Selectee, ActionType > &s, bool &hasAction_, OtherAction &action_ ) {
	    bool ha;
	    ActionType act;
	    return s.available( ha, act );
	} // Helper::available
    }; // Helper

    template< typename Selector, typename ActionType > bool nextAction( Selector &s, bool &hasAction_, ActionType &action_, uTime *timeout = NULL ) {
	SelectorStatus isAvailable;
	SelectorDL baseFuture;
	SelectorClient qs;

	baseFuture.client = &qs;
	hasAction_ = false;

	isAvailable = s.addAccept( &baseFuture, hasAction_, action_ );
	//std::osacquire( std::cerr ) << "nextAction: isAvailable " << isAvailable << " hasAction_ " << hasAction_ << std::endl;
	if ( isAvailable == GuardFalse ) {
	    if ( timeout != NULL ) {
		qs.sem.P( *timeout );
		return false;
	    } else {
		return true;
	    } // if
	} // if
	while ( isAvailable != Avail && ! hasAction_ ) {
	    //std::osacquire( std::cerr ) << "nextAction: isAvailable " << isAvailable << " hasAction_ " << hasAction_ << " sem count " << qs.sem.counter()<< std::endl;
	    if ( timeout != NULL ) {
		if ( ! qs.sem.P( *timeout ) ) goto fini; // timeout ?
	    } else {
		//std::osacquire( std::cerr ) << "nextAction: block " << qs.sem.counter() << std::endl;
		qs.sem.P();
		//std::osacquire( std::cerr ) << "nextAction: unblock " << qs.sem.counter() << std::endl;
	    } // if
	    isAvailable = s.available( hasAction_, action_ );
	    //std::osacquire( std::cerr ) << "nextAction: isAvailable " << isAvailable << " hasAction_ " << hasAction_ << " sem count " << qs.sem.counter()<< std::endl;
	} // while
      fini: ;
	//std::osacquire( std::cerr ) << "nextAction: exit loop " << isAvailable << " " << hasAction_ << std::endl;
	s.removeAccept( &baseFuture );
	return isAvailable == Avail;
    } // nextAction

    template< typename Selector, typename ActionType > bool nextAction( Selector &s, bool &hasAction_, ActionType &action_, uTime timeout ) {
	return nextAction( s, hasAction_, action_, &timeout );
    } // nextAction

    template< typename Selector, typename ActionType > bool tryNextAction( Selector &s, bool &hasAction_, ActionType &action_ ) {
	SelectorStatus isAvailable;
	hasAction_ = false;
	isAvailable = s.available( hasAction_, action_ );
	return isAvailable == Avail;
    } // tryNextAction

    // helper class for _Select statement implementation

    template< typename Selector >
    class Executor {
	Selector &selector;
	uTime timeout;
	bool hasTimeout, hasElse;

      public:
	enum { Done = 0, ElseAction = 1, TimeoutAction = 2 };

	Executor( Selector &selector_ ) : selector( selector_ ), hasTimeout( false ), hasElse( false ) {}
	Executor( Selector &selector_, bool elseGuard ) : selector( selector_ ), hasTimeout( false ), hasElse( elseGuard ) {}
	Executor( Selector &selector_, uTime timeout_ ) : selector( selector_ ), timeout( timeout_ ), hasTimeout( true ), hasElse( false ) {}
	Executor( Selector &selector_, uDuration timeout_ ) : selector( selector_ ), timeout( uThisProcessor().getClock().getTime() + timeout_ ), hasTimeout( true ), hasElse( false ) {}
	Executor( Selector &selector_, bool timeoutGuard, uTime timeout_ ) : selector( selector_ ), timeout( timeout_ ), hasTimeout( timeoutGuard ), hasElse( false ) {}
	Executor( Selector &selector_, bool timeoutGuard, uDuration timeout_ ) : selector( selector_ ), timeout( uThisProcessor().getClock().getTime() + timeout_ ), hasTimeout( timeoutGuard ), hasElse( false ) {}
	Executor( Selector &selector_, bool timeoutGuard, uTime timeout_, bool elseGuard ) : selector( selector_ ), timeout( timeout_ ), hasTimeout( timeoutGuard ), hasElse( elseGuard ) {}
	Executor( Selector &selector_, bool timeoutGuard, uDuration timeout_, bool elseGuard ) : selector( selector_ ), timeout( uThisProcessor().getClock().getTime() + timeout_ ), hasTimeout( timeoutGuard ), hasElse( elseGuard ) {}

	int nextAction() {
	    //std::osacquire( std::cerr ) << "Executor::nextAction enter" << std::endl;
	    int todo;
	    bool hasAction, isAvailable;
	    if ( hasElse ) {
		//std::osacquire( std::cerr ) << "Executor::nextAction hasElse" << std::endl;
		if ( tryNextAction( selector, hasAction, todo ) ) {
		    if ( hasAction ) {
			return todo;
		    } else {
			return Done;
		    } // if
		} else {
		    return ElseAction;
		} // if
	    } else {
		//std::osacquire( std::cerr ) << "Executor::nextAction no hasElse" << std::endl;
		isAvailable = UPP::nextAction( selector, hasAction, todo, hasTimeout ? &timeout : 0 );
		//std::osacquire( std::cerr ) << "Executor::nextAction isAvailable " << isAvailable << " hasAction " << hasAction << " hasTimeout " << hasTimeout << std::endl;
		if ( hasAction ) {
		    return todo;
		} else {
		    assert( hasTimeout || isAvailable );
		    return isAvailable ? Done : TimeoutAction;
		} // if
	    } // if
	} // Executor::nextAction
    }; // Executor
} // UPP

#endif // __U_BASESELECTOR_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
