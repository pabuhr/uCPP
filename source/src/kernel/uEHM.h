//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Russell Mok 1997
// 
// uEHM.h -- 
// 
// Author           : Russell Mok
// Created On       : Mon Jun 30 16:46:18 1997
// Last Modified By : Peter A. Buhr
// Last Modified On : Sun Sep 29 11:44:13 2024
// Update Count     : 553
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


#include <typeinfo>
#include <functional>

#define uRendezvousAcceptor uSerialMemberInstance.uAcceptor
enum { uEHMMaxMsg = 156 };
enum { uEHMMaxName = 100 };

#define uBaseEvent uBaseException

class uEHM;												// forward declaration


//######################### uBaseException ########################


class uBaseException {
	friend class uEHM;
	const std::type_info * getExceptionType() const { return &typeid( *this ); };
	virtual void stackThrow() const __attribute__(( noreturn )) = 0; // translator generated => object specific
  public:
	enum RaiseKind { ThrowRaise, ResumeRaise };
  protected:
	const uBaseCoroutine * src;							// source execution for async raise, set at raise
	char srcName[uEHMMaxName + 1];						//    and this field, too (+1 for string terminator)
	char msg[uEHMMaxMsg + 1];							// message to print if exception uncaught
	mutable void * boundObject;							// bound object for matching, set at raise
	mutable RaiseKind raiseKind;						// how the exception is raised

	uBaseException( const char * const msg = "" ) { src = nullptr; setMsg( msg ); }
	void setSrc( uBaseCoroutine & coroutine );
	void setMsg( const char * const msg );
  public:
	virtual ~uBaseException();

	const char * message() const { return msg; }
	const uBaseCoroutine & source() const { return *src; }
	const char * sourceName() const { return src != nullptr ? srcName : "*unknown*"; }
	RaiseKind getRaiseKind() const { return raiseKind; }
	const void * getRaiseObject() const { return boundObject; }
	const void * getOriginalThrower() const __attribute__(( deprecated )) { return getRaiseObject(); }
	const void * setRaiseObject( void * object ) { void * temp = boundObject; boundObject = object; return temp; }
	virtual void defaultTerminate();
	virtual void defaultResume();

	// These members should be private but cannot be because they are referenced from user code.
	virtual uBaseException * duplicate() const = 0;		// translator generated => object specific
	void reraise();
}; // uBaseException


//######################### uEHM ########################


class uEHM {
	friend class UPP::uKernelBoot;						// access: terminateHandler, unexpectedHandler
	friend class UPP::uMachContext;						// access: terminate
	friend class uBaseCoroutine;						// access: ResumeWorkHorseInit, uResumptionHandlers, uDeliverEStack, unexpected, strncpy
	friend class uBaseTask;								// access: terminateHandler
	friend class uBaseException;						// access: AsyncEMsg

	class ResumeWorkHorseInit;
	class AsyncEMsg;
	class AsyncEMsgBuffer;

	static bool match_exception_type( const std::type_info * derived_type, const std::type_info * parent_type );
	static bool deliverable_exception( const std::type_info * event_type );
	static void terminate() __attribute__(( noreturn ));
	static void terminateHandler() __attribute__(( noreturn ));
	static void unexpected() __attribute__(( noreturn ));
	static void unexpectedHandler() __attribute__(( noreturn ));
  public:
	enum FINALLY_CATCHRESUME_DISALLOW_RETURN { NORETURN }; // prevent returns in lambda body

	class uResumptionHandlers;							// usage generated by translator
	template< typename Functor > class uRoutineHandlerAny;
	template< typename Exn, typename Functor > class uRoutineHandler;
	class uFinallyHandler;

	class uHandlerBase;
	class uDeliverEStack;

	static void asyncToss( const uBaseException & event, uBaseCoroutine & target, uBaseException::RaiseKind raiseKind, bool rethrow = false );
	static void asyncReToss( uBaseCoroutine & target, uBaseException::RaiseKind raiseKind );

	static void Throw( const uBaseException & event, void * const bound = nullptr ) __attribute__(( noreturn ));
	//static void ThrowAt( const uBaseException & event, uBaseCoroutine & target ) { asyncToss( event, target, uBaseException::ThrowRaise ); }
	//static void ThrowAt( uBaseCoroutine & target ) { asyncReToss( target, uBaseException::ThrowRaise ); } // asynchronous rethrow
	static void ReThrow() __attribute__(( noreturn ));	// synchronous rethrow

	static void Resume( const uBaseException & event, void * const bound = nullptr, bool conseq = true );
	static void ReResume( bool conseq = true );
	static void ResumeAt( const uBaseException & event, uBaseCoroutine & target ) { asyncToss( event, target, uBaseException::ResumeRaise ); }
	static void ResumeAt( uBaseCoroutine & target ) { asyncReToss( target, uBaseException::ResumeRaise ); } // asynchronous reresume

	static bool pollCheck();
	static int poll();
	static const std::type_info * getTopResumptionType();
	static uBaseException * getCurrentException();
	static uBaseException * getCurrentResumption();
	static char * getCurrentExceptionName( uBaseException::RaiseKind raiseKind, char * s1, size_t n );
	static char * strncpy( char * s1, const char * s2, size_t n );
  private:
	static void resumeWorkHorse( const uBaseException & event, bool conseq );
}; // uEHM


//######################### uEHM::AsyncEMsg ########################


class uEHM::AsyncEMsg : public uSeqable {
	friend class uEHM;
	friend class uEHM::AsyncEMsgBuffer;
	//friend void uEHM::ThrowAt( const uBaseException &, uBaseCoroutine & );
	friend void uEHM::ResumeAt( const uBaseException &, uBaseCoroutine & );

	bool hidden;
	uBaseException * asyncException;

	AsyncEMsg & operator=( const AsyncEMsg & );
	AsyncEMsg( const AsyncEMsg & );

	AsyncEMsg( const uBaseException & event );
  public:
	~AsyncEMsg();
}; // uEHM::AsyncEMsg


//######################### uEHM::AsyncEMsgBuffer ########################


// AsyncEMsgBuffer looks like public uQueue<AsyncEMsg> but with mutex

class uEHM::AsyncEMsgBuffer : public uSequence<uEHM::AsyncEMsg> {
	AsyncEMsgBuffer( const AsyncEMsgBuffer & );
	AsyncEMsgBuffer& operator=( const AsyncEMsgBuffer & );
  public:
	uSpinLock lock;
	AsyncEMsgBuffer();
	~AsyncEMsgBuffer();
	void uAddMsg( AsyncEMsg * msg );
	AsyncEMsg * uRmMsg();
	AsyncEMsg * uRmMsg( AsyncEMsg * msg );
	AsyncEMsg * nextVisible( AsyncEMsg * msg );
}; // uEHM::AsyncEMsgBuffer


//######################### internal class and function declarations ########################


// base class allowing a list of otherwise-heterogeneous uHandlers
class uEHM::uHandlerBase {
	const void * const matchBinding;
	const std::type_info * eventType;
  protected:
	uHandlerBase( const void * matchBinding, const std::type_info * eventType ) : matchBinding( matchBinding ), eventType( eventType ) {}
	virtual ~uHandlerBase() {}
  public:
	virtual void uHandler( uBaseException & exn ) = 0;
	const void * getMatchBinding() const { return matchBinding; }
	const std::type_info * getExceptionType() const { return eventType; }
}; // uHandlerBase

template< typename Exn >
class uRoutineHandler : public uEHM::uHandlerBase {
	const std::function< uEHM::FINALLY_CATCHRESUME_DISALLOW_RETURN ( Exn & ) > handlerRtn; // lambda for exception handling routine
  public:
	uRoutineHandler( const std::function< uEHM::FINALLY_CATCHRESUME_DISALLOW_RETURN ( Exn & ) > & handlerRtn ) : uHandlerBase( nullptr, &typeid( Exn ) ), handlerRtn( handlerRtn ) {}
	uRoutineHandler( const void * originalThrower, const std::function< uEHM::FINALLY_CATCHRESUME_DISALLOW_RETURN ( Exn & ) > & handlerRtn ) : uHandlerBase( originalThrower, &typeid( Exn ) ), handlerRtn( handlerRtn ) {}
	virtual void uHandler( uBaseException & exn ) { handlerRtn( (Exn &)exn ); }
}; // uRoutineHandler

class uRoutineHandlerAny : public uEHM::uHandlerBase {
	const std::function< uEHM::FINALLY_CATCHRESUME_DISALLOW_RETURN () > handlerRtn; // lambda for exception handling routine
  public:
	uRoutineHandlerAny( const std::function< uEHM::FINALLY_CATCHRESUME_DISALLOW_RETURN () > & handlerRtn ) : uHandlerBase( nullptr, nullptr ), handlerRtn( handlerRtn ) {}
	virtual void uHandler( uBaseException & /* exn */ ) { handlerRtn(); }
}; // uRoutineHandlerAny


// Every set of resuming handlers bound to a template try block is saved in a uEHM::uResumptionHandlers object. The
// resuming handler hierarchy is implemented as a linked list.

class uEHM::uResumptionHandlers {
	friend void uEHM::resumeWorkHorse( const uBaseException &, bool );

	uResumptionHandlers * next, * conseqNext;			// uNext maintains a proper stack, while uConseqNext is used to skip
	// over handlers that have already been examined for resumption (to avoid recursion)

	const unsigned int size;							// number of handlers
	uHandlerBase * const * table;						// pointer to array of resumption handlers
  public:
	uResumptionHandlers( const uResumptionHandlers & ) = delete; // no copy
	uResumptionHandlers( uResumptionHandlers && ) = delete;
	uResumptionHandlers & operator=( const uResumptionHandlers & ) = delete; // no assignment
	uResumptionHandlers & operator=( uResumptionHandlers && ) = delete;

	uResumptionHandlers( uHandlerBase * const table[], const unsigned int size );
	~uResumptionHandlers();
}; // uEHM::uResumptionHandlers


// The following implements a linked list of event_id's table.  Used in enable and disable block.

class uEHM::uDeliverEStack {
	friend bool uEHM::deliverable_exception( const std::type_info * );

	uDeliverEStack * next;
	bool deliverFlag;									// true when events in table is Enable, otherwise false
	int  table_size;                                    // number of events in the table, 0 implies everything
	const std::type_info ** event_table;				// event id table
  public:
	uDeliverEStack( const uDeliverEStack & ) = delete;	// no copy
	uDeliverEStack( uDeliverEStack && ) = delete;
	uDeliverEStack & operator=( const uDeliverEStack & ) = delete; // no assignment
	uDeliverEStack & operator=( uDeliverEStack && ) = delete;

	uDeliverEStack( bool f, const std::type_info ** t = nullptr, unsigned int msg = 0 ); // for enable and disable blocks
	~uDeliverEStack();
}; // uEHM::uDeliverEStack


// Finally block is hoisted to lambda and invoked by RAII object nested in block surrounding "try" statement.

class uEHM::uFinallyHandler {
	const std::function< FINALLY_CATCHRESUME_DISALLOW_RETURN () > cleanUpRtn; // lambda for clean up
  public:
	uFinallyHandler( const std::function< FINALLY_CATCHRESUME_DISALLOW_RETURN () > & cleanUpRtn ) : cleanUpRtn( cleanUpRtn ) {}
	~uFinallyHandler()
	#if __cplusplus >= 201103L
	noexcept( false )									// C++11, required to allow exception from destructor
	#endif
		{
			try {
				cleanUpRtn();							// invoke handler on block exit
			} catch( ... ) {
				if ( std::__U_UNCAUGHT_EXCEPTION__() ) {
					abort( "Raising an exception in a _Finally clause during exception propagation is disallowed." );
				} else _Throw;
			} // try
		} // uFinallyHandler::~uFinallyHandler
}; // uEHM::uFinallyHandler


// Local Variables: //
// compile-command: "make install" //
// End: //
