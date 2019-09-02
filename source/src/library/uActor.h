//                              -*- Mode: C++ -*-
//
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr and Thierry Delisle 2016
//
// uActor.h --
//
// Author           : Peter A. Buhr and Thierry Delisle
// Created On       : Mon Nov 14 22:40:35 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Mar 15 08:28:57 2019
// Update Count     : 404
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


#pragma once


#include <uFuture.h>
#include <uSemaphore.h>

#ifndef Case
    #define Case( type, msg ) if ( type *msg##_d __attribute__(( unused )) = dynamic_cast< type * >( &msg ) )
#else
    #error actor provides a "Case" macro and macro name "Case" already in use.
#endif // ! Case

// http://doc.akka.io/docs/akka/current/java/untyped-actors.html


// Does not support parentage: all actors exist in flatland.

class uActor {
    static uExecutor * executor;			// executor for all actors
    static uSemaphore wait_;				// wait for all actors to delete
    static unsigned long int alive_;			// number of actor objects in system
  public:
    enum Allocation { Nodelete, Delete, Destroy, Finished }; // allocation actions
  private:
    unsigned long int ticket_;				// executor-queue handle to provide FIFO message execution
    Allocation allocation;				// allocation action
  public:
    struct Message {
	Allocation allocation;				// allocation action
	uActor * sender;				// delegated sender

	Message( Allocation allocation = Nodelete, uActor * sender = nullptr ) : allocation( allocation ), sender( sender ) {}
	Message( uActor * sender ) : allocation( Delete ), sender( sender ) {}
	virtual ~Message() {}
    }; // Message

    struct ReplyMsg : public Message {			// base future message
	ReplyMsg( Allocation allocation = Nodelete, uActor * sender = nullptr ) : Message( allocation, sender ) {}
	ReplyMsg( uActor * sender ) : Message( sender ) {}
	virtual bool available() = 0;
	virtual bool cancelled() = 0;
	virtual void cancel() = 0;
	virtual void delivery( uBaseEvent * ex ) = 0;
	virtual void reset() = 0;
    }; // ReplyMsg

    template< typename Result > struct FutureMessage : public ReplyMsg {
	Future_ISM< Result > result;

	FutureMessage( Allocation allocation = Nodelete, uActor * sender = nullptr ) : ReplyMsg( allocation, sender ) {}
	FutureMessage( uActor * sender ) : ReplyMsg( Delete, sender ) {}
	bool available() { return result.available(); }
	bool cancelled() { return result.cancelled(); }
	void cancel() { result.cancel(); }
	void delivery( Result res ) { result.delivery( res ); } // raises uDelivered
	void delivery( uBaseEvent * ex ) { result.delivery( ex ); } // raises uDelivered
	void reset() { result.reset(); }
    }; // FutureMessage
  private:
    static inline void lastActor() {
	if ( uFetchAdd( alive_, -1 ) == 1 ) wait_.V();	// 1 => count is zero
    } // lastActor

    static inline void checkMsg( Message & msg ) {
	switch ( msg.allocation ) {			// analyze message status
	  case Nodelete: break;
	  case Delete: delete &msg; break;
	  case Destroy: msg.~Message(); break;
	  case Finished: break;
	} // switch
    } // checkMsg

    struct Deliver_ {
	uActor & actor;
	Message & msg;

	Deliver_( uActor & actor, Message & msg ) : actor( actor ), msg( msg ) {}

	void operator()() {				// functor
	    try {
		actor.allocation = actor.process_( msg ); // call current message handler
		switch ( actor.allocation ) {		// analyze actor status
		  case Nodelete: break;
		  case Delete: delete &actor; break;
		  case Destroy: actor.~uActor(); break;
		  case Finished: lastActor(); break;
		} // switch
	    } catch ( uBaseEvent &ex ) {
		Case( uActor::ReplyMsg, msg ) {		// unknown future message
		    msg_d->delivery( ex.duplicate() );	// complain in future
		} else {
		    checkMsg( msg );			// process message
		    _Throw;				// fail to worker thread
		} // Case
	    } catch ( ... ) {
		abort( "C++ exceptions unsupported from throw in actor for future message" );
	    // To have a zero-cost try block, the checkMsg call is duplicated above/below.
	    // } _Finally {
	    // 	checkMsg( msg );			// process message
	    } // try
	    checkMsg( msg );				// process message
	} // Deliver_::operator()
    }; // Deliver_

    virtual Allocation process_( Message & msg ) = 0;	// type-safe access to subclass receivePtr
  protected:
    // Do NOT make pure to allow replacement by "become" in constructor.
    virtual Allocation receive( Message & ) {		// user supplied message handler
	abort( "must supply receive routine for actor" );
	return Delete;
    };
    template< typename Func > void send_( Func action ) { executor->send( action, ticket_ ); }
    virtual void preStart() { /* default empty */ };	// user supplied actor initialization

    struct uActorConstructor {				// translator creates instance in actor constructor
	uActorConstructor( UPP::uAction action, uActor &actor ) {
	    if ( action == UPP::uYes ) {
		actor.send_( [&actor]() { actor.preStart(); } ); // send preStart call
	    } // if
	} // uActorConstructor::uActorConstructor
    }; // uActorConstructor
  public:
    uActor() {
	uFetchAdd( alive_, 1 );				// number of actors in system
	uDEBUG( if ( ! executor ) { abort( "Attempt to create actor but no actor executor exists.\nPossible cause is not calling uActorStart() or calling it to late." ); } );
	ticket_ = executor->tickets();			// get executor queue handle
    } // uActor::uActor

    virtual ~uActor() {
	if ( allocation != Finished ) lastActor();	// check for last actor ?
    } // uActor::~uActor

    // Communication

    uActor & tell( Message & msg, uActor * sender = nullptr ) { // async call, no return
	msg.sender = sender;
	executor->send( Deliver_( *this, msg ), ticket_ ); // copy functor
	return *this;
    } // uActor::tell

    uActor & operator|( Message & msg ) {		// operator async call, no return
	return tell( msg, nullptr );
    } // uActor::operator|

    template< typename Result > Future_ISM< Result > ask( FutureMessage< Result > & msg, uActor * sender = nullptr ) { // async call, return future
	msg.sender = sender;
	executor->send( Deliver_( *this, msg ), ticket_ );
	return msg.result;
    } // uActor::ask

    template< typename Result > Future_ISM< Result > operator||( FutureMessage< Result > &msg ) { // operator async call, return future
	return ask( msg, nullptr );
    } // uActor::operator||

    // Administration

#   define uActorStart() uExecutor __uExecutor__; uActor::start( __uExecutor__ )
    static void start( uExecutor & executor ) {		// wait for all actors to terminate or timeout
	assert( ! uActor::executor );
	uActor::executor = &executor;
    } // uActor::start

#   define uActorStop() uActor::stop()
    static bool stop( uDuration duration = 0 ) {	// wait for all actors to terminate or timeout
	if ( duration == 0 ) {				// optimization
	    uActor::wait_.P();
	    return true;				// true => stop
	} else {
	    return uActor::wait_.P( duration );		// true => stop, false => timeout
	} // if
    } // uActor::stop

    static struct StartMsg : public uActor::Message {} startMsg; // start actor
    static struct StopMsg : public uActor::Message {} stopMsg; // terminate actor

    // Error handling

    static struct UnhandledMsg : public uActor::Message {} unhandledMsg; // tell error

    _Event Unhandled {					// ask error
      public:
	Message * msg;
	Unhandled( Message * msg ) : msg( msg ) {}
    }; // uActor::Unhandled
}; // uActor


template< typename Actor > class uActorType : public uActor {
  protected:
    typedef Allocation (Actor:: * Handler)( Message & msg ); // message handler type
  private:
    Handler receivePtr_ = &uActorType<Actor>::receive;	// message handler pointer

    virtual Allocation process_( Message & msg ) override final {
	return (((Actor *)this)->*receivePtr_)(msg);
    } // uActorType::process

    void restart_() {
	receivePtr_ = &uActorType<Actor>::receive;	// restart message-handler pointer
	preStart();					// rerun preStart
    } // uActorType::restart_
  protected:
    // Must be done from within the actor not from outside the actor.
    // Does not provide a stack of message handlers => no "unbecome".
    Handler become( Handler handler ) {			// dynamically change message handler
	Handler temp = receivePtr_;
	receivePtr_ = handler;
	return temp;					// return previous message handler
    } // uActorType::become
  public:
    // Administration

    void restart() {					// reset actor to initial state
	send_( [this]() { this->restart_(); } );	// run restart message
    } // uActorType::restart
}; // uActorType



// Local Variables: //
// compile-command: "make install" //
// End: //
