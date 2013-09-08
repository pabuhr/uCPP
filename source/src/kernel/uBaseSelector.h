//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Richard C. Bilson 2007
// 
// uBaseSelector.h -- 
// 
// Author           : Richard C. Bilson
// Created On       : Sat Jul 14 07:25:52 2007
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Dec 10 21:40:47 2009
// Update Count     : 16
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

namespace UPP {
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
	} // signal
    }; // SelectorDL

    class OrCondition {
      public:
	static bool sat( bool left, bool right ) {
	    //cerr << "AndCondition::sat selectee " << selectee << " next " << next << " result " << ( selectee || next ) << "\n";
	    return left || right;
	} // operator()

	static bool shortcircuit( bool left ) {
	    return left;
	} // shortcircuit
    }; // OrCondition

    class AndCondition {
      public:
	static bool sat( bool left, bool right ) {
	    //cerr << "AndCondition::sat selectee " << selectee << " next " << next << " result " << ( selectee && next ) << "\n";
	    return left && right;
	} // operator()

	static bool shortcircuit( bool left ) {
	    return false;
	} // shortcircuit
    }; // AndCondition

    enum SelectorStatus { NAvail = false, Avail = true, GuardFalse };

    template< typename Selectee > struct Helper;
    template< typename Condition, typename Left, typename Right, typename ActionType > class BinarySelector;

    // UnarySelector serves three purposes:
    // - attaches an action to a selectable
    // - attaches a guard to a selectable
    // - provides a copyable wrapper for a non-copyable selectable (e.g., Future_ESM)
    template< typename Selectee, typename ActionType >
	class UnarySelector {
	typedef UnarySelector< Selectee, ActionType > ThisSelector;
	template< typename S > friend struct Helper;
	Selectee &selectee;
	ActionType action;
	bool guard;
	bool doRemove, hasAction;
	SelectorDL baseFuture;

	  public:
	UnarySelector( const Selectee &q, ActionType act ) : selectee( const_cast< Selectee& >( q ) ), action( act ), guard( true ), doRemove( false ), hasAction( true ) {
	} // UnarySelector

	UnarySelector( bool guard_, const Selectee &q, ActionType act ) : selectee( const_cast< Selectee& >( q ) ), action( act ), guard( guard_ ), doRemove( false ), hasAction( true ) {
	} // UnarySelector

	UnarySelector( const Selectee &q ) : selectee( const_cast< Selectee& >( q ) ), guard( true ), doRemove( false ), hasAction( false ) {
	} // UnarySelector

	UnarySelector( bool guard_, const Selectee &q ) : selectee( const_cast< Selectee& >( q ) ), guard( guard_ ), doRemove( false ), hasAction( false ) {
	} // UnarySelector

	ThisSelector addAction( const ActionType &act ) const {
	    ThisSelector ret( *this );
	    ret.action = act;
	    return ret;
	}

	ThisSelector addGuard( bool guard ) const {
	    ThisSelector ret( *this );
	    ret.guard = guard;
	    return ret;
	}

	SelectorStatus addAccept( SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
	    //cerr << "unary addAccept " << this << " guard " << guard << endl;
	    if ( !guard ) return GuardFalse;
	    SelectorStatus child = Helper< Selectee >::addAccept( selectee, acceptState, hasAction_, action_ );
	    if ( ( child == Avail ) && !hasAction_ && hasAction ) {
		hasAction_ = true;
		//cerr << "addAccept assigning action " << action << endl;
		action_ = action;
		hasAction = false;
	    } // if
	    doRemove = ( child == NAvail );
	    //cerr << "addAccept " << this << " returns " << child << endl;
	    return child;
	} // addAccept

	void removeAccept( SelectorDL *acceptState ) {
	    if ( doRemove ) {
		selectee.removeAccept( acceptState );
		doRemove = false;
	    } // if
	} // removeAccept

	SelectorStatus available( bool &hasAction_, ActionType &action_ ) {
	    if ( !guard ) return GuardFalse;
	    SelectorStatus child = Helper< Selectee >::available( selectee, hasAction_, action_ );
	    if ( ( child == Avail ) && !hasAction_ && hasAction ) {
		hasAction_ = true;
		//cerr << "available assigning action " << action << endl;
		action_ = action;
		hasAction = false;
	    } // if
	    return child;
	} // available

	template< typename Other >
	    BinarySelector< OrCondition, ThisSelector, UnarySelector< Other, int >, int > operator||( const Other &s2 ) {
	    return BinarySelector< OrCondition, ThisSelector, UnarySelector< Other, int >, int >( *this, UnarySelector< Other, int >( s2 ) );
	} // operator||

	template< typename Other >
	    BinarySelector< AndCondition, ThisSelector, UnarySelector< Other, int >, int > operator&&( const Other &s2 ) {
	    return BinarySelector< AndCondition, ThisSelector, UnarySelector< Other, int >, int >( *this, UnarySelector< Other, int >( s2 ) );
	} // operator&&
    }; // UnarySelector

    // Left and Right must be copyable and selectable types 
    template< typename Condition, typename Left, typename Right, typename ActionType >
	class BinarySelector {
	typedef BinarySelector< Condition, Left, Right, ActionType > ThisSelector;
	template< typename S > friend struct Helper;
	template< typename C, typename L, typename R, typename A > friend BinarySelector< C, L, R, A > &when( bool cond, BinarySelector< C, L, R, A > &selectee );

	Left left;
	Right right;
	ActionType action;
	bool guard;

	bool removeLeft, removeRight, hasAction;
	SelectorDL baseFuture;

	  public:
	BinarySelector( const Left &left_, const Right &right_, ActionType act ) : left( left_ ), right( right_ ), action( act ), guard( true ), removeLeft( false ), removeRight( false ), hasAction( true ) {
	} // BinarySelector

	BinarySelector( bool guard_, const Left &left_, const Right &right_, ActionType act ) : left( left_ ), right( right_ ), action( act ), guard( guard_ ), removeLeft( false ), removeRight( false ), hasAction( true ) {
	} // BinarySelector

	BinarySelector( const Left &left_, const Right &right_ ) : left( left_ ), right( right_ ), guard( true ), removeLeft( false ), removeRight( false ), hasAction( false ) {
	} // BinarySelector

	BinarySelector( bool guard_, const Left &left_, const Right &right_ ) : left( left_ ), right( right_ ), guard( guard_ ), removeLeft( false ), removeRight( false ), hasAction( false ) {
	} // BinarySelector

	ThisSelector addAction( ActionType act ) const {
	    ThisSelector ret( *this );
	    ret.action = act;
	    return ret;
	}

	ThisSelector addGuard( bool guard ) const {
	    ThisSelector ret( *this );
	    ret.guard = guard;
	    return ret;
	}

	SelectorStatus addAccept( SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
	    //cerr << "binary addAccept " << this << " guard " << guard << endl;
	    if ( !guard ) return GuardFalse;
	    SelectorStatus sat;
	    //cerr << "binary addAccept " << this << " left child " << &left << endl;
	    SelectorStatus leftstat = Helper< Left >::addAccept( left, acceptState, hasAction_, action_ );
	    removeLeft = ( leftstat == NAvail );
	    if ( Condition::shortcircuit( leftstat == Avail ) ) {
		sat = Avail;
	    } else {
		baseFuture.client = acceptState->client;
		//cerr << "binary addAccept " << this << " right child " << &right << endl;
		SelectorStatus rightstat = Helper< Right >::addAccept( right, &baseFuture, hasAction_, action_ );
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
	    }
	    if ( ( sat == Avail ) && ! hasAction_ && hasAction ) {
		hasAction = false;
		hasAction_ = true;
		//cerr << "addAccept assigning action " << action << endl;
		action_ = action;
	    } // if
	    if ( sat != NAvail ) {
		removeAccept( acceptState );
	    } // if
	    //cerr << "addAccept " << this << " returns " << sat << endl;
	    return sat;
	} // addAccept

	void removeAccept( SelectorDL *acceptState ) {
	    //cerr << "removeAccept enter " << this << " next " << &next << endl;
	    if ( !guard ) return;
	    if ( removeLeft ) {
		left.removeAccept( acceptState );
		removeLeft = false;
	    } // if
	    if ( removeRight ) {
		right.removeAccept( &baseFuture );
		removeRight = false;
	    } // if
	} // removeAccept

	SelectorStatus available( bool &hasAction_, ActionType &action_ ) {
	    if ( !guard ) return GuardFalse;
	    // it is necessary here to inquire specifically about the guard of the left child, rather than simply
	    // recursing, because the recursion could return true either because the guard is false or because the guard
	    // is true and the sub-expression is satisfied. It is important to be able to distinguish these two cases to
	    // decide when to short-circuit.
	    SelectorStatus sat;
	    SelectorStatus leftstat = Helper< Left >::available( left, hasAction_, action_ );
	    if ( Condition::shortcircuit( leftstat == Avail ) ) {
		sat = Avail;
	    } else {
		//cerr << "addAccept " << this << " right " << &right << endl;
		SelectorStatus rightstat = Helper< Right >::available( right, hasAction_, action_ );
		//cerr << "addAccept " << this << " doRemove " << doRemove << " leftAvailable " << leftAvailable << endl;
		if ( leftstat == GuardFalse && rightstat == GuardFalse ) {
		    sat = GuardFalse;
		} else if ( leftstat == GuardFalse ) {
		    sat = rightstat;
		} else if ( rightstat == GuardFalse ) {
		    sat = leftstat;
		} else {
		    sat = (SelectorStatus)Condition::sat( (bool)leftstat, (bool)rightstat );
		} // if
	    }
	    if ( ( sat == Avail ) && ! hasAction_ && hasAction ) {
		hasAction = false;
		hasAction_ = true;
		//cerr << "available assigning action " << action << endl;
		action_ = action;
	    } // if
	    return sat;
	} // available

	template< typename Other >
	    BinarySelector< OrCondition, ThisSelector, UnarySelector< Other, int >, int > operator||( const Other &s2 ) {
	    return BinarySelector< OrCondition, ThisSelector, UnarySelector< Other, int >, int >( *this, UnarySelector< Other, int >( s2 ) );
	} // operator||

	template< typename Other >
	    BinarySelector< AndCondition, ThisSelector, UnarySelector< Other, int >, int > operator&&( const Other &s2 ) {
	    return BinarySelector< AndCondition, ThisSelector, UnarySelector< Other, int >, int >( *this, UnarySelector< Other, int >( s2 ) );
	} // operator&&

    }; // BinarySelector

    template< typename Selectee > struct Helper {
	template< typename ActionType > static SelectorStatus addAccept( Selectee &s, SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
	    return (SelectorStatus)!s.addAccept( acceptState );
	} // addAccept

	template< typename ActionType > static SelectorStatus available( Selectee &s, bool &hasAction_, ActionType &action_ ) {
	    return (SelectorStatus)s.available();
	} // available
    }; // Helper

    template< typename Condition, typename Left, typename Right, typename ActionType >
	struct Helper< BinarySelector< Condition, Left, Right, ActionType > > {
	    static SelectorStatus addAccept( BinarySelector< Condition, Left, Right, ActionType > &s, SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
		return s.addAccept( acceptState, hasAction_, action_ );
	    } // addAccept

	    template< typename OtherAction > static SelectorStatus addAccept( BinarySelector< Condition, Left, Right, ActionType > &s, SelectorDL *acceptState, bool &hasAction_, OtherAction &action_ ) {
		bool ha;
		ActionType act;
		return s.addAccept( acceptState, ha, act );
	    } // addAccept

	    static SelectorStatus available( BinarySelector< Condition, Left, Right, ActionType > &s, bool &hasAction_, ActionType &action_ ) {
		return s.available( hasAction_, action_ );
	    } // available

	    template< typename OtherAction > static SelectorStatus available( BinarySelector< Condition, Left, Right, ActionType > &s, bool &hasAction_, OtherAction &action_ ) {
		bool ha;
		ActionType act;
		return s.available( ha, act );
	    } // available
	}; // Helper

    template< typename Selectee, typename ActionType >
	struct Helper< UnarySelector< Selectee, ActionType > > {
	    static SelectorStatus addAccept( UnarySelector< Selectee, ActionType > &s, SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
		return s.addAccept( acceptState, hasAction_, action_ );
	    } // addAccept

	    template< typename OtherAction > static SelectorStatus addAccept( UnarySelector< Selectee, ActionType > &s, SelectorDL *acceptState, bool &hasAction_, ActionType &action_ ) {
		bool ha;
		ActionType act;
		return s.addAccept( acceptState, ha, act );
	    } // addAccept

	    static SelectorStatus available( UnarySelector< Selectee, ActionType > &s, bool &hasAction_, ActionType &action_ ) {
		return s.available( hasAction_, action_ );
	    } // available

	    template< typename OtherAction > static SelectorStatus available( UnarySelector< Selectee, ActionType > &s, bool &hasAction_, OtherAction &action_ ) {
		bool ha;
		ActionType act;
		return s.available( ha, act );
	    } // available
	}; // Helper

    template< typename Selector, typename ActionType >
	bool nextAction( Selector &s, bool &hasAction_, ActionType &action_, uTime *timeout = NULL ) {
	SelectorStatus isAvailable;
	SelectorDL baseFuture;
	SelectorClient qs;
	baseFuture.client = &qs;
	hasAction_ = false;
	//cerr << "nextAction " << this << " addAccept " << &baseFuture << endl;
	isAvailable = s.addAccept( &baseFuture, hasAction_, action_ );
	if ( isAvailable == GuardFalse ) {
	    if ( timeout != NULL ) {
		qs.sem.P( *timeout );
		return false;
	    } else {
		return true;
	    } // if
	} // if
	//cerr << "nextAction isAvailable " << isAvailable << " hasAction_ " << hasAction_ << endl;
	while ( ( isAvailable != Avail ) && ! hasAction_ ) {
	    //cerr << "no short circuit, count " << qs.sem.counter() << endl;
	    if ( timeout != NULL ) {
		if ( ! qs.sem.P( *timeout ) ) goto fini;
	    } else {
		qs.sem.P();
	    }
	    isAvailable = s.available( hasAction_, action_ );
	} // while
	  fini:
	s.removeAccept( &baseFuture );
	return isAvailable == Avail;
    }

    template< typename Selector, typename ActionType >
	bool nextAction( Selector &s, bool &hasAction_, ActionType &action_, uTime timeout ) {
	return nextAction( s, hasAction_, action_, &timeout );
    }

    template< typename Selector, typename ActionType >
	bool tryNextAction( Selector &s, bool &hasAction_, ActionType &action_ ) {
	SelectorStatus isAvailable;
	hasAction_ = false;
	isAvailable = s.available( hasAction_, action_ );
	return isAvailable == Avail;
    }

    // helper class for _Select statement implementation

    template< typename Selector >
	class Executor {
	Selector &selector;
	uTime timeout;
	bool hasTimeout, hasElse;

      public:
	enum { Done = 0, ElseAction = 1, TimeoutAction = 2 };

	Executor( Selector &selector_ ) : selector( selector_ ), hasTimeout( false ), hasElse( false ) {}
	Executor( Selector &selector_,  bool elseGuard ) : selector( selector_ ), hasTimeout( false ), hasElse( elseGuard ) {}
	Executor( Selector &selector_, uTime timeout_ ) : selector( selector_ ), timeout( timeout_ ), hasTimeout( true ), hasElse( false ) {}
	Executor( Selector &selector_, uDuration timeout_ ) : selector( selector_ ), timeout( uThisProcessor().getClock().getTime() + timeout_ ), hasTimeout( true ), hasElse( false ) {}
	Executor( Selector &selector_, bool timeoutGuard, uTime timeout_ ) : selector( selector_ ), timeout( timeout_ ), hasTimeout( timeoutGuard ), hasElse( false ) {}
	Executor( Selector &selector_, bool timeoutGuard, uDuration timeout_ ) : selector( selector_ ), timeout( uThisProcessor().getClock().getTime() + timeout_ ), hasTimeout( timeoutGuard ), hasElse( false ) {}
	Executor( Selector &selector_, bool timeoutGuard, uTime timeout_, bool elseGuard ) : selector( selector_ ), timeout( timeout_ ), hasTimeout( timeoutGuard ), hasElse( elseGuard ) {}
	Executor( Selector &selector_, bool timeoutGuard, uDuration timeout_, bool elseGuard ) : selector( selector_ ), timeout( uThisProcessor().getClock().getTime() + timeout_ ), hasTimeout( timeoutGuard ), hasElse( elseGuard ) {}

	int nextAction() {
	    int todo;
	    bool hasAction, isAvailable;
	    if ( hasElse ) {
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
		isAvailable = UPP::nextAction( selector, hasAction, todo, hasTimeout ? &timeout : 0 );
		if ( hasAction ) {
		    return todo;
		} else {
		    assert( hasTimeout || isAvailable );
		    return isAvailable ? Done : TimeoutAction;
		} // if
	    } // if
	} // nextAction
    }; // Executor
} // UPP

#endif // __U_BASESELECTOR_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
