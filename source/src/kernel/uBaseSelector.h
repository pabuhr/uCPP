//                              -*- Mode: C++ -*-
//
// uC++ Version 6.1.0, Copyright (C) Jingge Fu 2015
//
// uBaseSelector.h --
//
// Author           : Jingge Fu
// Created On       : Sat Jul 14 07:25:52 2007
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Jul 11 23:42:07 2015
// Update Count     : 172
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
    // The complexity of this algorithm stems from need to support all possible executable statements in the statement
    // after a _Select clause, e.g., lexical accesses and direct control transfers. Lambda expressions cannot handle
    // gotos because labels have routine scope. As a result, the evaluation of the _Select expression must keep
    // returning to the root of the tree so the statement of a select clauses can be executed, and then the tree must be
    // traversed again to determine when the expression is complete. To mitigate having to restart the tree walk, nodes
    // in the tree that have triggered execution of a select clause are pruned so the tree walk is shortened for each
    // selected clause.

    // The evaluation tree is composed of unary and binary nodes, and futures. A unary node represents a future or
    // binary node, with an optional action from the _Select clause. A unary node represents a binary node for this
    // case:
    //
    //    _Select( f1 || f2 ) A;
    //
    // because the expression A binary node represents a logical operation (|| or &&) relating unary or future nodes.

    //  U(A)  U(A)      B(no A)
    //   |     |         /   \.
    //   F     B        U|B U|B

    // Tree nodes are built and chained together on the stack of the thread executing the _Select statement, so there is
    // no dynamic allocation. The code is generalized to allow potential use in other situations.


    // State represents the availabilty of a node; when the root is available, the _Select ends.  When a leaf node has a
    // WhenCondition, it returns GuardFalse.  It is up to the parent node to interpret GuardFalse.  Note there are only
    // three possibilities.  If the parent node is an binary node with type "and", GuardFalse is interpreted as Avail.
    // If the parent node is an binary node with type "or", GuardFalse is interpreted as NonAvail.  If there is no
    // parent node (_Select on a single node), GuardFalse is interpreted as Avail.
    enum State { Avail, NonAvail, GuardFalse, };

    struct BaseFutureDL : public uSeqable {		// interface class containing blocking sem.
	virtual void signal() = 0;
	virtual ~BaseFutureDL() {}
    };  // BaseFutureDL


    struct SelectorClient {				// wrapper around the sem
	uSemaphore sem;					// selection client waits if no future available
	SelectorClient() : sem( 0 ) {};
    };  // SelectorClient


    // Each UnarySelector owns a SelectorDL, it is used for registering. 
    struct SelectorDL : public BaseFutureDL {
	SelectorClient *client;				// client data for server
	virtual ~SelectorDL() {}
	virtual void signal() { client->sem.V(); }	// SelectorDL::signal
    }; // SelectorDL


    struct Condition {					// represent And and Or condition
	enum Type { And, Or };

	static State sat( State &left, State &right, Type type ) {
	    if ( left == GuardFalse ) {			// process left branch ?
		left = type == And ? Avail : NonAvail;
	    } // if

	    if ( right == GuardFalse ) {
		right = type == And ? Avail : NonAvail;
	    } // if

	    bool result;
	    if ( type == And ) {			// simple condition
		result = (left == Avail) && (right == Avail);
	    } else {
		result = (left == Avail) || (right == Avail);
	    } // if

	    return result ? Avail: NonAvail;
	} // Condition::sat
    }; // Condition


    // UnarySelector is a leaf containing a future. 
    template <typename Future>
    class UnarySelector {
	int myAction;					// action #
	Future &selectee;				// future
	UPP::SelectorDL bench;				// bench it registers with
	bool WhenCondition;				// _When clause present
	bool remove;					// need to remove bench from future
      public:
	UnarySelector( Future &q ) : myAction( 0 ), selectee( q ), WhenCondition( true ), remove( false ) {};
	UnarySelector( Future &q, int act ) : myAction( act ), selectee( q ), WhenCondition( true ),remove( false ) {};
	UnarySelector( bool guard, Future &q, int act ) : myAction( act ), selectee( q ), WhenCondition( guard ), remove( false ) {};

	~UnarySelector() {
	    if ( remove ) { 
                selectee.removeAccept( &bench );
                remove = false;
	    } // if
	} // UnarySelector::~UnarySelector

	// Register with futrue
	void registerSelf( UPP::SelectorClient *acceptState ) {
	    bench.client = acceptState;
	    remove = !selectee.addAccept( &bench );
	} // UnarySelector::registerSelf

	// Give out action
	State nextAction( bool &hasAction, int &action ) {
	    if ( ! WhenCondition ) return GuardFalse;

	    if ( myAction ) {
		if ( ! hasAction && selectee.available() ) {
		    hasAction = true;
		    action = myAction;
		    myAction = 0;
		    if ( remove ) {
                        selectee.removeAccept( &bench );
                        remove = false;
		    } // if
		    return Avail;
		} else {				// unavailable or action already inserted
		    return NonAvail;
		} // if
	    } else {					// action already executed
		return Avail;
	    } // if
	} // UnarySelector:nextAction

	void setAction( int action ) {
	    assert( myAction == 0 );
	    myAction = action;
	} // UnarySelector:setAction
    }; // UnarySelector


    // BinarySelector represents conditions and, or
    template <typename Left, typename Right>
    class BinarySelector{
	Left left;					// left child
	Right right;					// right child
	State leftStatus;				// left child status
	State rightStatus;				// right child status
	Condition::Type type;				// condition
      public:
	BinarySelector( const Left &left, const Right &right, Condition::Type type )
	    	: left( left ), right( right ), leftStatus( NonAvail ), rightStatus( NonAvail ), type( type ) {}

	void registerSelf( UPP::SelectorClient *acceptState ) {
	    left.registerSelf( acceptState );
	    right.registerSelf( acceptState );
	} // BinarySelector::registerSelf

	State nextAction( bool &hasAction, int &action ) {
	    leftStatus = left.nextAction( hasAction, action );
	    if ( hasAction ) {
		return Condition::sat( leftStatus, rightStatus, type );
	    } else {
		rightStatus = right.nextAction( hasAction, action );
		return Condition::sat( leftStatus, rightStatus, type );
	    } // if
	} // BinarySelector::nextAction

	void setAction( int action ) {
	    left.setAction( action );
	    right.setAction( action );
	} // BinarySelector::setAction
    }; // BinarySelector


    // Specialization for future expression, e.g., _Select( f1 || f2 )
    template <> template < typename Left, typename Right >
    class UnarySelector< BinarySelector< Left, Right > > {
	BinarySelector< Left, Right > selectee;
      public:
	UnarySelector( const BinarySelector<Left, Right> &b, int act ) : selectee( b ) {
	    selectee.setAction( act );
	} // UnarySelector::UnarySelector

	void registerSelf( UPP::SelectorClient *acceptState ) {
	    selectee.registerSelf( acceptState );
	} // UnarySelector::registerSelf

	State nextAction( bool &hasAction, int &action ) {
	    return selectee.nextAction( hasAction, action );
	} // UnarySelector::nextAction

	void setAction( int action )  {
	    assert( false );				// should not be called
	    return;
	} // UnarySelector::setAction
    }; // UnarySelector


    // Executor holds tree root, and evaluates tree and gets next action.
    template<typename Root>
    class Executor {
	Root &root;
	uTime timeout;
	bool hasTimeout, hasElse;
	State isFinish;
	UPP::SelectorClient acceptState;
      public:
	Executor( Root &root ) : root( root ), hasTimeout( false ), hasElse( false ), isFinish( NonAvail ) {
	    root.registerSelf( &acceptState );
	}

	Executor( Root &root, bool elseGuard ) : root( root ), hasTimeout( false ), hasElse( elseGuard ), isFinish( NonAvail ) {
	    root.registerSelf( &acceptState );
	}

	Executor( Root &root, uTime timeout ) : root( root ), timeout( timeout ), hasTimeout( true ), hasElse( false ), isFinish( NonAvail ) {
	    root.registerSelf( &acceptState );
	}

	Executor( Root &root, uDuration timeout ) : root( root ), timeout( uThisProcessor().getClock().getTime() + timeout ), hasTimeout( true ), hasElse( false ), isFinish( NonAvail ) {
	    root.registerSelf( &acceptState );
	}

	Executor( Root &root, bool timeoutGuard, uTime timeout ) : root( root ), timeout( timeout ), hasTimeout( timeoutGuard ), hasElse( false ), isFinish( NonAvail ) {
	    root.registerSelf( &acceptState );
	}

	Executor( Root &root, bool timeoutGuard, uDuration timeout )
		: root( root ), timeout( uThisProcessor().getClock().getTime() + timeout ), hasTimeout( timeoutGuard ), hasElse( false ), isFinish( NonAvail ) {
	    root.registerSelf( &acceptState );
	}

	Executor( Root &root, bool timeoutGuard, uTime timeout, bool elseGuard )
		: root( root ), timeout( timeout ), hasTimeout( timeoutGuard ), hasElse( elseGuard ), isFinish( NonAvail ) {
	    root.registerSelf( &acceptState );
	}

	Executor( Root &root, bool timeoutGuard, uDuration timeout, bool elseGuard )
		: root( root ), timeout( uThisProcessor().getClock().getTime() + timeout ), hasTimeout( timeoutGuard ), hasElse( elseGuard ), isFinish( NonAvail ) {
	    root.registerSelf( &acceptState );
	}

	// Get the next action, if the whole expression is true, return 0.
	// If it has _Else, return 1;
	// If it has _TimeOut, return 2;
	int nextAction() {
	    if ( isFinish == Avail ) return 0;

	    bool hasAction = false;
	    int action;
	    for ( ;; ) {
		isFinish = root.nextAction( hasAction, action );
		if ( hasAction ) {
		    return action;
		} else if ( hasElse ) {
		    return 1;
		} else if ( hasTimeout ) {
		    bool wakeup = acceptState.sem.P( timeout );
		    if ( wakeup == false ) {
			return 2;
		    } // if 
		} else if ( isFinish == Avail ) {
		    // This happens when there is _When clause at the end.
		    return 0;
		} else {
		    acceptState.sem.P();
		} // if
	    } // for
	} // Executor::nextAction
    }; // Executor
} // UPP

#endif  // __U_BASESELECTOR_H__

// Local Variables: //
// compile-command: "make install" //
// End: //
