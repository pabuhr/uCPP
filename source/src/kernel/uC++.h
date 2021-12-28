//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uC++.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Dec 17 22:04:27 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Mon Dec 27 17:13:11 2021
// Update Count     : 6070
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

// ***************************************************************************
// WARNING: If a .cc file is added to the kernel that does not define
// __U_KERNEL__ BEFORE including uC++.h, the profiler will likely fail to
// register processors created using the uDefaultProcessors mechanism.
// Furthermore, no explicit error is reported in this case -- the problem can
// only be detected by observing erroneous results reported by certain metrics
// that are sensitive to the registration of processors.
// ***************************************************************************


#pragma once


// #pragma clang diagnostic push
// #pragma clang diagnostic ignored "-Wnull-dereference"

#if __GNUC__ >= 7										// valid GNU compiler diagnostic ?
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"	// Mute g++-7
#endif // __GNUC__ >= 7
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wnull-dereference"


#if __cplusplus >= 201703L								// c++17 ?
#define __U_UNCAUGHT_EXCEPTION__ uncaught_exceptions
#else
#define __U_UNCAUGHT_EXCEPTION__ uncaught_exception
#endif // __cplusplus >= 201703L


#ifdef KNOT
// remove select FD checking
#undef _FORTIFY_SOURCE
#define _FORTIFY_SOURCE 0
#endif // KNOT

// see uDebug.h to activate
#define uDEBUGPRT( stmt )

#ifdef __U_DEBUG__
#define uDEBUG( stmt ) stmt
#else
#define uDEBUG( stmt )
#endif // __U_DEBUG__

#if defined( __U_MULTI__ )
#   define __U_THREAD__ __thread
#else
#   define __U_THREAD__
#endif // __U_MULTI__

#include <sys/types.h>									// select, fd_set

// The GNU Libc defines C library functions with throw () when compiled under C++, to enable optimizations.  When uC++
// overrides these functions, it must provide identical exception specifications.

#define __THROW throw ()
	// Certain library functions have had __THROW removed from their prototypes to support the NPTL implementation of
	// pthread cancellation. To compile with a pre-NPTL version of the header files use
	//
	//   #define __OLD_THROW throw ()
#define __OLD_THROW

#if ! defined( _LIBC_REENTRANT )
#   define _LIBC_REENTRANT
#endif
#include <cerrno>

#include <cstddef>										// ptrdiff_t
#include <cstdlib>										// malloc, calloc, realloc, free
#include <malloc.h>										// memalign
#include <link.h>										// dl_iterate_phdr
#include <csignal>										// signal, etc.
#include <sys/mman.h>									// mmap
#include <ucontext.h>									// ucontext_t

#include <exception>
#include <iosfwd>										// std::filebuf
#include <pthread.h>									// PTHREAD_CANCEL_*
#include <unwind-cxx.h>									// struct __cxa_eh_globals

#include <assert.h>

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

//######################### InterposeSymbol #########################


namespace UPP {
	class RealRtn {
		static void * interposeSymbol( const char * symbolName, const char * version = nullptr );
	  public:
		static void startup();

		static __typeof__( ::exit ) * exit __attribute__(( noreturn ));
		static __typeof__( ::abort ) * abort __attribute__(( noreturn ));
		static __typeof__( ::pselect ) * pselect;
		static __typeof__( std::set_terminate ) * set_terminate;
		static __typeof__( std::set_unexpected ) * set_unexpected;
		static __typeof__( ::dl_iterate_phdr ) * dl_iterate_phdr;
		static __typeof__( ::pthread_create ) * pthread_create;
//		static __typeof__( ::pthread_exit ) * pthread_exit;
		static __typeof__( ::pthread_attr_init ) * pthread_attr_init;
		static __typeof__( ::pthread_attr_setstack ) * pthread_attr_setstack;
		static __typeof__( ::pthread_kill ) * pthread_kill;
		static __typeof__( ::pthread_join ) * pthread_join;
		static __typeof__( ::pthread_self ) * pthread_self;
		static __typeof__( ::pthread_setaffinity_np ) * pthread_setaffinity_np;
		static __typeof__( ::pthread_getaffinity_np ) * pthread_getaffinity_np;
	}; // RealRtn
} // UPP


//######################### start uC++ code #########################


#include <uAlign.h>
#include <uStack.h>
#include <uQueue.h>
#include <uSequence.h>
#include <uBitSet.h>
#include <uDefault.h>

#include "uKernelThreads.h"
#include "uAtomic.h"

// C-heap allocation extensions
extern "C" {
	void * aalloc( size_t dim, size_t elemSize ) __THROW;
	void * resize( void * oaddr, size_t size ) __THROW;
	void * amemalign( size_t align, size_t dim, size_t elemSize ) __THROW;
	void * cmemalign( size_t alignment, size_t noOfElems, size_t elemSize ) __THROW;
	size_t malloc_alignment( void * addr ) __THROW;
	bool malloc_zero_fill( void * addr ) __THROW;
	size_t malloc_size( void * addr ) __THROW;
	size_t malloc_dimension( void * addr ) __THROW;
	size_t malloc_usable_size( void * addr ) __THROW;
	void malloc_stats() __THROW;
	int malloc_stats_fd( int fd ) __THROW;
} // extern "C"
// Must have C++ linkage to overload with C linkage realloc.
void * resize( void * oaddr, size_t alignment, size_t size ) __THROW;
void * realloc( void * addr, size_t alignment, size_t size ) __THROW;

#if defined( __U_MULTI__ )
	typedef pthread_t uPid_t;
#else
	typedef pid_t uPid_t;
#endif // __U_MULTI__

// supported mallopt options
#ifndef M_MMAP_THRESHOLD
#define M_MMAP_THRESHOLD (-1)
#endif // M_TOP_PAD
#ifndef M_TOP_PAD
#define M_TOP_PAD (-2)
#endif // M_TOP_PAD


#ifdef __U_STATISTICS__
namespace UPP {
	struct Statistics {
		// Kernel, signed because of the atomic inc/dec
		static long int ready_queue, spins, spin_sched, mutex_queue, mutex_lock_queue, owner_lock_queue, adaptive_lock_queue, io_lock_queue;
		static long int uSpinLocks, uLocks, uMutexLocks, uOwnerLocks, uCondLocks, uSemaphores, uSerials;

		// I/O statistics
		static unsigned long int select_syscalls, select_errors, select_eintr;
		static unsigned long int select_events, select_nothing, select_blocking, select_pending;
		static unsigned long int select_maxFD;
		static unsigned long int accept_syscalls, accept_errors;
		static unsigned long int read_syscalls, read_errors, read_eagain, read_chunking, read_bytes;
		static unsigned long int write_syscalls, write_errors, write_eagain, write_bytes;
		static unsigned long int sendfile_syscalls, sendfile_errors, sendfile_eagain, first_sendfile, sendfile_yields;

		static unsigned long int iopoller_exchange, iopoller_spin;
		static unsigned long int signal_alarm, signal_usr1;

		// Scheduling statistics
		static unsigned long int coroutine_context_switches;
		static unsigned long int roll_forward;
		static unsigned long int user_context_switches;
		static unsigned long int kernel_thread_yields, kernel_thread_pause;
		static unsigned long int wake_processor;
		static unsigned long int events, setitimer;
	  private:
		static bool prtStatTerm_;						// print statistics on termination signal
	  public:
		static bool prtStatTerm() {
			return prtStatTerm_;
		} // prtStatTerm

		static bool prtStatTermOn() {
			bool temp = prtStatTerm_;
			prtStatTerm_ = true;
			return temp;
		} // prtStatTermOn

		static bool prtStatTermOff() {
			bool temp = prtStatTerm_;
			prtStatTerm_ = false;
			return temp;
		} // prtStatTermOff

		static void print();
	}; // Statistics
} // UPP
#endif // __U_STATISTICS__


#define _Monitor _Mutex class							// short form for monitor
#define _Cormonitor _Mutex _Coroutine					// short form for coroutine monitor

_Task uSystemTask;										// forward declaration
class uBaseCoroutine;									// forward declaration
class __attribute__(( may_alias )) uBaseTask;			// forward declaration
class uBaseSpinLock;									// forward declaration
class __attribute__(( may_alias )) uSpinLock;			// forward declaration
class uLock;											// forward declaration
class __attribute__(( may_alias )) uOwnerLock;			// forward declaration
class uCondLock;										// forward declaration
class uProcessor;										// forward declaration
class uDefaultScheduler;								// forward declaration
class uCluster;											// forward declaration
_Task uProcessorTask;									// forward declaration
class uEventList;										// forward declaration
_Task uPthreadable;										// forward declaration
class HWCounters;										// forward declaration
_Task uLocalDebugger;									// forward declaration
struct uIOClosure;										// forward declaration
class uCondition;										// forward declaration
class uTimeoutHndlr;									// forward declaration
class uWakeupHndlr;										// forward declaration
class uRWLock;											// forward declaration

namespace UPP {
	enum  uAction { uNo, uYes };						// forward declaration
	class uKernelBoot;									// forward declaration
	class uInitProcessorsBoot;							// forward declaration
	_Task uBootTask;									// forward declaration
	class uHeapManager;									// forward declaration
	class uHeapControl;									// forward declaration
	class uSerial;										// forward declaration
	class uSerialConstructor;							// forward declaration
	class uSerialDestructor;							// forward declaration
	class uSerialMember;								// forward declaration
	class uMachContext;									// forward declaration
	_Task uPthread;										// forward declaration
	class PthreadLock;									// forward declaration
	_Coroutine uProcessorKernel;						// forward declaration
	class uNBIO;										// forward declaration
	void umainProfile();								// forward declaration
} // UPP

extern uBaseCoroutine & uThisCoroutine();				// forward declaration
extern uBaseTask & uThisTask();							// forward declaration
extern uProcessor & uThisProcessor();					// forward declaration
extern uCluster & uThisCluster();						// forward declaration


//######################### Profiling ########################


_Task uProfiler;										// forward declaration
class uProfilerBoot;
class uProfileTaskSampler;
class uProfileClusterSampler;
class uProfileProcessorSampler;

extern "C" {
	void __cyg_profile_func_enter( void * pcCurrentFunction, void * pcCallingFunction );
	void __cyg_profile_func_exit( void * pcCurrentFunction, void * pcCallingFunction );
} // extern "C"

#ifdef __U_PROFILER__
#define __U_HW_OVFL_SIG__ SIGIO
#endif // __U_PROFILER__


//######################### Signal Handling #########################


// define parameter types for signal handlers

#define __U_SIGCXT__ ucontext_t *
#define __U_SIGPARMS__ int sig __attribute__(( unused )), siginfo_t * sfp __attribute__(( unused )), __U_SIGCXT__ cxt __attribute__(( unused ))
#define __U_SIGTYPE__ int, siginfo_t *, __U_SIGCXT__

namespace UPP {
	class uSigHandlerModule {
		friend class uKernelBoot;						// access: uSigHandlerModule
		friend _Task ::uLocalDebugger;					// access: signal
#ifdef __U_PROFILER__
		friend _Task ::uProfiler;						// access: signal, signalContextPC
#endif // __U_PROFILER__

		static sigset_t block_mask;						// block all signals

		static void signal( int sig, void (* handler)(__U_SIGPARMS__), int flags = 0 );
		static void sigAlrmHandler( __U_SIGPARMS__ );

		uSigHandlerModule( const uSigHandlerModule & ) = delete; // no copy
		uSigHandlerModule( uSigHandlerModule && ) = delete;
		uSigHandlerModule & operator=( const uSigHandlerModule & ) = delete; // no assignment
		uSigHandlerModule & operator=( uSigHandlerModule && ) = delete;

		uSigHandlerModule();
	  public:
		enum SignalAbort { Yes, No };
	}; // uSigHandlerModule
} // UPP


//######################### abnormal exit #########################


extern void exit( int retcode, const char fmt[], ... ) __THROW __attribute__(( format( printf, 2, 3 ), __nothrow__, __leaf__, __noreturn__ ));
extern void abort( UPP::uSigHandlerModule::SignalAbort signalAbort, const char fmt[], ... ) __attribute__(( format( printf, 2, 3 ), __nothrow__, __leaf__, noreturn ));
extern void abort( const char fmt[], ... ) __attribute__(( format( printf, 1, 2 ), __nothrow__, __leaf__, noreturn ));
namespace std {
	using ::abort;										// needed for replacing std::stream routines
}


//######################### uProcessor #########################


class uProcessorDL : public uSeqable {
	uProcessor & processor_;
  public:
	uProcessorDL( uProcessor & processor_ ) : processor_( processor_ ) {}
	uProcessor & processor() const { return processor_; }
}; // uProcessorDL

typedef uSequence<uProcessorDL> uProcessorSeq;


//######################### uCluster #########################


class uClusterDL : public uSeqable {
	uCluster & cluster_;
  public:
	uClusterDL( uCluster & cluster_ ) : cluster_( cluster_ ) {}
	uCluster & cluster() const { return cluster_; }
}; // uClusterDL

typedef uSequence<uClusterDL> uClusterSeq;


//######################### uBaseTask #########################


class uBaseTaskDL : public uSeqable {
	uBaseTask & task_;
  public:
	uBaseTaskDL( uBaseTask & task_ ) : task_( task_ ) {}
	uBaseTask & task() const { return task_; }
}; // uBaseTaskDL

typedef uSequence<uBaseTaskDL> uBaseTaskSeq;


//######################### uKernelModule #########################


#ifdef __U_PROFILER__
extern "C" {											// TEMPORARY: profiler allocating memory from the kernel issue
	int pthread_mutex_lock( pthread_mutex_t * mutex ) __THROW;
	int pthread_mutex_trylock( pthread_mutex_t * mutex ) __THROW;
	int pthread_mutex_unlock( pthread_mutex_t * mutex ) __THROW;
} // extern "C"
#endif // __U_PROFILER__


class uKernelModule {
	friend void uAbort( UPP::uSigHandlerModule::SignalAbort signalAbort, const char fmt[], va_list args ); // access: globalAbort
	friend void exit( int retcode ) __THROW;			// access: globalAbort
	friend class UPP::uSigHandlerModule;				// access: uKernelModuleBoot, rollForward
	friend class UPP::uMachContext;						// access: everything
	friend class uBaseCoroutine;						// access: uKernelModuleBoot
	friend class uBaseTask;								// access: uKernelModuleBoot
	friend class UPP::uSerial;							// access: uKernelModuleBoot
	friend class uBaseSpinLock;							// access: uKernelModuleBoot
	friend class uMutexLock;							// access: uKernelModuleBoot, initialized
	friend class uOwnerLock;							// access: uKernelModuleBoot, initialized
	template< int, int, int > friend class uAdaptiveLock; // access: uKernelModuleBoot, initialized
	friend class uCondLock;								// access: uKernelModuleBoot
	friend class uContext;								// access: uKernelModuleBoot
	friend _Coroutine UPP::uProcessorKernel;			// access: uKernelModuleBoot, globalProcessors, globalClusters, systemProcessor
	friend class uProcessor;							// access: everything
	friend uBaseTask & uThisTask();						// access: uKernelModuleBoot
	friend uProcessor & uThisProcessor();				// access: uKernelModuleBoot
	friend uCluster & uThisCluster();					// access: uKernelModuleBoot
	friend _Task uProcessorTask;						// access: uKernelModuleBoot
	friend class uCluster;								// access: uKernelModuleBoot, globalClusters, globalClusterLock, rollForward
	friend _Task UPP::uBootTask;						// access: uKernelModuleBoot, systemCluster
	friend _Task uSystemTask;							// access: systemCluster
	friend void UPP::umainProfile();					// access: bootTask
	friend class UPP::uKernelBoot;						// access: everything
	friend class UPP::uInitProcessorsBoot;				// access: numUserProcessors, userProcessors
	friend class UPP::uHeapManager;						// access: bootTaskStorage, kernelModuleInitialized, startup
	friend class UPP::uNBIO;							// access: uKernelModuleBoot
	friend int pthread_mutex_lock( pthread_mutex_t * mutex ) __THROW; // access: kernelModuleInitialized

	// real-time

	friend class uEventList;							// access: uKernelModuleBoot
	friend class uEventListPop;							// access: uKernelModuleBoot

	// debugging

	friend _Task uLocalDebugger;						// access: uKernelModuleBoot, bootTask, globalClusters, systemProcessor, systemCluster
	friend class uLocalDebuggerHandler;					// access: uKernelModuleBoot

#ifdef __U_PROFILER__
	// profiling

	friend class UPP::PthreadLock;						// access: initialized
	friend int pthread_mutex_trylock( pthread_mutex_t * mutex ) __THROW; // TEMPORARY: profiler allocating memory from the kernel issue
	friend int pthread_mutex_unlock( pthread_mutex_t * mutex ) __THROW; // TEMPORARY: profiler allocating memory from the kernel issue
	friend void __cyg_profile_func_enter( void * pcCurrentFunction, void * pcCallingFunction ); // access: uKernelModuleBoot
	friend void __cyg_profile_func_exit( void * pcCurrentFunction, void * pcCallingFunction ); // access: uKernelModuleBoot
	friend class uProfiler;								// access: uKernelModuleBoot
	friend class uProfilerBoot;							// access: uKernelModuleBoot, bootTask, systemCluster, userProcessor
	friend class HWCounters;							// access: uKernelModuleBoot
	friend class CGMonitor;								// access: uKernelModuleBoot
	template<typename Elem, unsigned int BlockSize, typename Admin> friend struct uFixedListArray; // access: uKernelModuleBoot
#endif // __U_PROFILER__

	friend class uKernelSampler;						// access: globalClusters
	friend class uClusterSampler;						// access: globalClusters
	friend __typeof__( ::dl_iterate_phdr ) dl_iterate_phdr; // access: disableInterrupts, enableInterrupts

	struct uKernelModuleData {
		volatile uKernelModuleData * This;

#		define __U_MAX_PAGE_SIZE__ (16 * 1024)
#		define activeProcessorKernel (THREAD_GETMEM( processorKernelStorage ))
		uProcessor * activeProcessor;					// current active processor
		// The next two private variables shadow the corresponding fields in the processor data structure. They are an
		// optimization so that routines uThisCluster and uThisTask do not have to be atomic routines, and as a
		// consequence can be inlined. The problem is the multiple activeProcessor variables (one per UNIX process). A
		// task executing on one processor can be time sliced after loading the address of the active processor into a
		// register, rescheduled on another processor and restarted, but now the previously loaded processor pointer is
		// incorrect. By updating these two shadow variables whenever the corresponding processor field is changed (and
		// this occurs atomically in the kernel), the appropriate data structure (cluster or task) can be accessed with
		// a single load instruction, which is atomic.
		uCluster * activeCluster;						// current active cluster for processor
		uBaseTask * activeTask;							// current active task for processor

		bool disableInt;								// task in kernel: no time slice interrupts
		int disableIntCnt;

		bool disableIntSpin;							// task in spin lock; no time slice interrupts
		int disableIntSpinCnt;

		bool RFinprogress;								// roll forward in progress
		bool RFpending;									// roll forward pending and needs execution

		UPP::uProcessorKernel * processorKernelStorage;	// system-cluster processor kernel

		// The thread pointer value needs to be accessible so that it can be properly restored on context switches.  On
		// a non-tls system the thread pointer points directly at the kernel module, i.e. tp == This.  On a tls system
		// the system places the kernel module, so tp != This.

		// unsigned long threadPointer;

		void disableInterrupts() volatile {
			THREAD_SETMEM( disableInt, true );
			int old = THREAD_GETMEM( disableIntCnt );
			THREAD_SETMEM( disableIntCnt, old + 1 );
		} // uKernelModule::uKernelModuleData::disableInterrupts

		void enableInterrupts() volatile {
			uDEBUG( assert( disableInt && disableIntCnt > 0 ); )

			disableIntCnt -= 1;							// decrement number of disablings
			if ( disableIntCnt == 0 ) {
				disableInt = false;						// enable interrupts
				if ( ! THREAD_GETMEM( RFinprogress ) && THREAD_GETMEM( RFpending ) && ! THREAD_GETMEM( disableIntSpin ) ) { // rollForward callable ?
					rollForward();
				} // if
			} // if
			uDEBUG( assert( ( ! THREAD_GETMEM( disableInt ) && THREAD_GETMEM( disableIntCnt ) == 0 ) || ( THREAD_GETMEM( disableInt ) && THREAD_GETMEM( disableIntCnt ) > 0 ) ); )
		} // KernelModule::uKernelModuleData::enableInterrupts

		void disableIntSpinLock() volatile {
			uDEBUG( assert( ( ! THREAD_GETMEM( disableIntSpin ) && THREAD_GETMEM( disableIntSpinCnt ) == 0 ) || ( THREAD_GETMEM( disableIntSpin ) && THREAD_GETMEM( disableIntSpinCnt ) > 0 ) ); )

			THREAD_SETMEM( disableIntSpin, true );
			int old = THREAD_GETMEM( disableIntSpinCnt ); // processor independent increment
			THREAD_SETMEM( disableIntSpinCnt, old + 1 );

			uDEBUG( assert( THREAD_GETMEM( disableIntSpin ) && THREAD_GETMEM( disableIntSpinCnt ) > 0 ); )
		} // uKernelModule::uKernelModuleData::disableIntSpinLock

		void enableIntSpinLock() volatile {
			uDEBUG( assert( THREAD_GETMEM( disableIntSpin ) && THREAD_GETMEM( disableIntSpinCnt ) > 0 ); )

			disableIntSpinCnt -= 1;						// decrement number of disablings
			if ( disableIntSpinCnt == 0 ) {
				disableIntSpin = false;					// enable interrupts

				if ( ! THREAD_GETMEM( RFinprogress ) && THREAD_GETMEM( RFpending ) && ! THREAD_GETMEM( disableInt ) ) {		// rollForward callable ?
					rollForward();
				} // if
			} // if

			uDEBUG( assert( ( ! THREAD_GETMEM( disableIntSpin ) && THREAD_GETMEM( disableIntSpinCnt ) == 0 ) || ( THREAD_GETMEM( disableIntSpin ) && THREAD_GETMEM( disableIntSpinCnt ) > 0 ) ); )
		} // uKernelModule::uKernelModuleData::enableIntSpinLock

		void enableIntSpinLockNoRF() volatile {
			uDEBUG( assert( THREAD_GETMEM( disableIntSpin ) && THREAD_GETMEM( disableIntSpinCnt ) > 0 ); )

			disableIntSpinCnt -= 1;						// decrement number of disablings
			if ( disableIntSpinCnt == 0 ) {
				disableIntSpin = false;					// enable interrupts
			} // if

			uDEBUG( assert( ( ! THREAD_GETMEM( disableIntSpin ) && THREAD_GETMEM( disableIntSpinCnt ) == 0 ) || ( THREAD_GETMEM( disableIntSpin ) && THREAD_GETMEM( disableIntSpinCnt ) > 0 ) ); )
		} // uKernelModule::uKernelModuleData::enableIntSpinLock

		void ctor() volatile;							// POD constructor
	}; // uKernelModuleData


	// shared, initialized in uC++.cc

	static bool kernelModuleInitialized;
	static volatile __U_THREAD__ uKernelModuleData uKernelModuleBoot;
	uDEBUG( static bool initialized; )					// initialization/finalization incomplete
#if __U_LOCALDEBUGGER_H__
	static unsigned int attaching;						// flag to signal the local kernel to start attaching.
#endif // __U_LOCALDEBUGGER_H__
#ifndef __U_MULTI__
	static bool deadlock;								// deadlock detected in kernel
#endif // ! __U_MULTI__
	static bool globalAbort;							// indicate aborting processor
	static bool globalSpinAbort;						// indicate aborting processor to spin locks
	static uSpinLock * globalAbortLock;					// only one aborting processors
	static uSpinLock * globalProcessorLock;				// mutual exclusion for global processor operations
	static uProcessorSeq * globalProcessors;			// global list of processors
	static uSpinLock * globalClusterLock;				// mutual exclusion for global cluster operations
	static uClusterSeq * globalClusters;				// global list of cluster
	static uDefaultScheduler * systemScheduler;			// pointer to systen scheduler for system cluster
	static uProcessor * systemProcessor;				// pointer to system processor
	static UPP::uBootTask * bootTask;					// pointer to boot task for global constructors/destructors
	static uProcessor ** userProcessors;				// pointer to user processors
	static unsigned int numUserProcessors;				// number of user processors
	static char systemProcessorStorage[];
	static char systemClusterStorage[];
	static uCluster * systemCluster;						// pointer to system cluster
	static uCluster * userCluster;						// pointer to user cluster
	static char bootTaskStorage[];

	static std::filebuf * cerrFilebuf, * clogFilebuf, * coutFilebuf, * cinFilebuf;

	static void rollForward( bool inKernel = false );
	static void * startThread( void * p );

	static void abortExit();
	static void startup();								// init boot KM
  public:
	static uSystemTask * systemTask;					// pointer to system task for global constructors/destructors

	static bool afterMain;
}; // uKernelModule


inline uProcessor & uThisProcessor() {
	return *THREAD_GETMEM( activeProcessor );
} // uThisProcessor


inline uCluster & uThisCluster() {
	return *THREAD_GETMEM( activeCluster );
} // uThisCluster


inline uBaseTask & uThisTask() {
	return *THREAD_GETMEM( activeTask );
} // uThisTask


//######################### uSpinLock #########################


class uBaseSpinLock {									// non-yielding spinlock
	friend class UPP::uKernelBoot;						// access: new
	friend class uEventListPop;							// access: acquire_, release_
	friend class uCluster;								// access: value

	unsigned int value;

	void acquire_( bool rollforward );

	void release_( bool rollforward ) {
		assert( value != 0 );
		uTestReset( value );
		if ( rollforward ) {							// allow timeslicing during spinning
			THREAD_GETMEM( This )->enableIntSpinLockNoRF();
		} else {
			THREAD_GETMEM( This )->enableIntSpinLock();
		} // if
	} // uBaseSpinLock::release_
  public:
	uBaseSpinLock( const uBaseSpinLock & ) = delete;	// no copy
	uBaseSpinLock( uBaseSpinLock && ) = delete;
	uBaseSpinLock & operator=( const uBaseSpinLock & ) = delete; // no assignment
	uBaseSpinLock & operator=( uBaseSpinLock && ) = delete;

	uBaseSpinLock() {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::uSpinLocks, 1 );
#endif // __U_STATISTICS__
		value = 0;										// unlock
	} // uBaseSpinLock::uBaseSpinLock

	void acquire() {
		acquire_( false );
		asm( "" : : : "memory" );						// prevent code movement across barrier
	} // uBaseSpinLock::acquire

	bool tryacquire();

	void release() {
		asm( "" : : : "memory" );						// prevent code movement across barrier
		release_( false );
	} // uBaseSpinLock::release
}; // uBaseSpinLock


class uSpinLock : public uBaseSpinLock {				// handle alignment to prevent false sharing
//	char padding[128 - sizeof(uBaseSpinLock)];			// pad to size of cacheline
  public:
	void * operator new( size_t size ) {				// dynamic allocation
//		return ::memalign( 128, size );
		return ::malloc( size );
	} // uSpinLock::operator new
}; // __attribute__(( aligned (128) ));					// static allocation


// RAII mutual-exclusion lock.  Useful for mutual exclusion in free routines.  Handles exception termination and
// multiple block exit or return.

class uCSpinLock {
	uSpinLock & spinLock;
  public:
	uCSpinLock( const uCSpinLock & ) = delete;			// no copy
	uCSpinLock( uCSpinLock && ) = delete;
	uCSpinLock & operator=( const uCSpinLock & ) = delete; // no assignment
	uCSpinLock & operator=( uCSpinLock && ) = delete;

	uCSpinLock( uSpinLock & spinLock ) : spinLock( spinLock ) {
		spinLock.acquire();
	} // uCSpinLock::uCSpinLock

	~uCSpinLock() {
		spinLock.release();
	} // uCSpinLock::~uCSpinLock
}; // uCSpinLock


//######################### uLock #########################


class uLock {											// yielding spinlock
	uSpinLock spinLock;									// must be first field for alignment
	unsigned int value;
  public:
	uLock( const uLock & ) = delete;					// no copy
	uLock( uLock && ) = delete;
	uLock & operator=( const uLock & ) = delete;		// no assignment
	uLock & operator=( uLock && ) = delete;

	uLock() {
		value = 1;
	} // uLock::uLock

	uLock( unsigned int val ) {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::uLocks, 1 );
#endif // __U_STATISTICS__
		uDEBUG(
			if ( val > 1 ) {
				abort( "Attempt to initialize uLock %p to %d that exceeds range 0-1.", this, val );
			} // if
		)
		value = val;
	} // uLock::uLock

	void acquire();
	bool tryacquire();

	void release() {
		value = 1;
	} // uLock::release

	void * operator new( size_t size ) {
		return ::memalign( 128, size );					// size of cache line to prevent false sharing
	} // uLock::operator new
}; // uLock


//######################### Abnormal Event Handling #########################


#include <uEHM.h>


_Event uKernelFailure {									// general event for kernel failures, inherit implicitly from uBaseEvent (exception root)
  protected:
	uKernelFailure( const char * const msg = "" );
  public:
	virtual ~uKernelFailure();
	virtual void defaultTerminate() override;
}; // uKernelFailure


_Event uMutexFailure : public uKernelFailure {			// general event for mutex member failures
	const UPP::uSerial * const serial;					// identify the mutex object of _Cormonitor or _Task
  protected:
	uMutexFailure( const UPP::uSerial * const serial, const char * const msg = "" );
	uMutexFailure( const char * const msg = "" );
  public:
	const UPP::uSerial * serialId() const;

	// exception handling

	_Event EntryFailure;
	_Event RendezvousFailure;
}; // uMutexFailure


_Event uMutexFailure::EntryFailure : public uMutexFailure {
  public:
	EntryFailure( const UPP::uSerial * const serial, const char * const msg = "" );
	EntryFailure( const char * const msg = "" );
	virtual ~EntryFailure();
	virtual void defaultTerminate() override;
}; // uMutexFailure::EntryFailure


_Event uMutexFailure::RendezvousFailure : public uMutexFailure {
	const uBaseCoroutine * const caller_;
  public:
	RendezvousFailure( const UPP::uSerial * const serial, const char * const msg = "" );
	virtual ~RendezvousFailure();
	const uBaseCoroutine * caller() const;
	virtual void defaultTerminate() override;
}; // uMutexFailure::RendezvousFailure


_Event uIOFailure {										// general event for IO failures, inherit implicitly from uBaseEvent (exception root)
	int errno_;
  protected:
	uIOFailure( int errno__, const char * const msg );
  public:
	virtual ~uIOFailure();
	int errNo() const;
}; // uIOFailure


//######################### Real-Time #########################


#include <uCalendar.h>


class uSignalHandler : public uColable {
  protected:
	uBaseTask * This;
	virtual ~uSignalHandler() {}
  public:
	uBaseTask * getThis() { return This; }
	virtual void handler() = 0;
}; // uSignalHandler


#include <uAlarm.h>


class uCxtSwtchHndlr : public uSignalHandler {
	friend class uProcessor;							// access: constructor
	friend class uEventListPop;							// access: processor

#if defined( __U_MULTI__ )
	uProcessor & processor;
	uCxtSwtchHndlr( uProcessor & processor ) : processor( processor ) {}
#endif // __U_MULTI__
	void handler();
}; // uCxtSwtchHndlr


//######################### uMutexLock #########################

// uMutexLock/uOwnerLock use wait morphing with uCondLock. uCondLock removes an unblocking task and moves it to the lock
// queue for processing through the add_ member in the locks. Wait morphing eliminates the double blocking if the
// unblocking thread must reacquire the mutex lock after the wait.

class uMutexLock {
	friend class uCondLock;								// access: add_, release_
  protected:
	// These data fields must be initialized to zero. Therefore, this lock can be used in the same storage area as a
	// pthread_mutex_t, if sizeof(pthread_mutex_t) >= sizeof(uMutexLock).

	uBaseSpinLock spinLock;								// must be first field for alignment
	unsigned int count;									// number of recursive entries; no overflow checking
	uSequence<uBaseTaskDL> waiting;						// sequence versus queue to reduce size to 24 bytes => more expensive

	virtual void add_( uBaseTask & task );				// helper routines for uCondLock
	void release_();
  public:
	uMutexLock( const uMutexLock & ) = delete;			// no copy
	uMutexLock( uMutexLock && ) = delete;
	uMutexLock & operator=( const uMutexLock & ) = delete; // no assignment
	uMutexLock & operator=( uMutexLock && ) = delete;

	uMutexLock() {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::uMutexLocks, 1 );
#endif // __U_STATISTICS__
		count = false;									// no one has acquired the lock
	} // uMutexLock::uMutexLock

	uDEBUG( ~uMutexLock(); )

	void acquire();
	bool tryacquire();
	void release();

	void * operator new( size_t size ) {
		return ::operator new( size );
	} // uMutexLock::operator new

	void * operator new( size_t, void * storage ) {		// used in pthread_mutex
		return storage;
	} // uMutexLock::operator new
}; // uMutexLock


//######################### uOwnerLock #########################


class uOwnerLock : public uMutexLock {
	friend class uCondLock;								// access: add_, release_

	// These data fields must be initialized to zero. Therefore, this lock can be used in the same storage area as a
	// pthread_mutex_t, if sizeof(pthread_mutex_t) >= sizeof(uOwnerLock).

	uBaseTask * owner_;									// owner with respect to recursive entry

	void add_( uBaseTask & task );						// helper routines for uCondLock
	void release_();
  public:
	uOwnerLock( const uOwnerLock & ) = delete;			// no copy
	uOwnerLock( uOwnerLock && ) = delete;
	uOwnerLock & operator=( const uOwnerLock & ) = delete; // no assignment
	uOwnerLock & operator=( uOwnerLock && ) = delete;

	uOwnerLock() {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::uOwnerLocks, 1 );
#endif // __U_STATISTICS__
		owner_ = nullptr;								// no one owns the lock
		count = 0;										// so count is zero
	} // uOwnerLock::uOwnerLock

	uDEBUG( ~uOwnerLock(); )

	unsigned int times() const {
		return count;
	} // uOwnerLock::times

	uBaseTask * owner() const {
		return owner_;
	} // uOwnerLock::times

	void acquire();
	bool tryacquire();
	void release();

	void * operator new( size_t size ) {
		return ::operator new( size );
	} // uOwnerLock::operator new

	void * operator new( size_t, void * storage ) {		// used in pthread_mutex
		return storage;
	} // uOwnerLock::operator new
}; // uOwnerLock


//######################### uCondLock #########################


class uCondLock {
	struct TimedWaitHandler : public uSignalHandler {	// real-time
		uCondLock & condlock;
		bool timedout;

		TimedWaitHandler( uBaseTask & task, uCondLock & condlock );
		TimedWaitHandler( uCondLock & condlock );
		void handler();
	}; // TimedWaitHandler

	// These data fields must be initialized to zero. Therefore, this lock can be used in the same storage area as a
	// pthread_cond_t, if sizeof(pthread_cond_t) >= sizeof(uCondLock).

	uBaseSpinLock spinLock;								// must be first field for alignment
	uSequence<uBaseTaskDL> waiting;						// queue of blocked tasks
	void waitTimeout( TimedWaitHandler & h );			// timeout
  public:
	uCondLock( const uCondLock & ) = delete;			// no copy
	uCondLock( uCondLock && ) = delete;
	uCondLock & operator=( const uCondLock & ) = delete; // no assignment
	uCondLock & operator=( uCondLock && ) = delete;

	uCondLock() {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::uCondLocks, 1 );
#endif // __U_STATISTICS__
	} // uCondLock::uCondLock

	uDEBUG( ~uCondLock(); )
	void wait( uMutexLock & lock );
	void wait( uMutexLock & lock, uintptr_t info );
	bool wait( uMutexLock & lock, uDuration duration );
	bool wait( uMutexLock & lock, uintptr_t info, uDuration duration );
	bool wait( uMutexLock & lock, uTime time );
	bool wait( uMutexLock & lock, uintptr_t info, uTime time );
	void wait( uOwnerLock & lock );
	void wait( uOwnerLock & lock, uintptr_t info );
	bool wait( uOwnerLock & lock, uDuration duration );
	bool wait( uOwnerLock & lock, uintptr_t info, uDuration duration );
	bool wait( uOwnerLock & lock, uTime time );
	bool wait( uOwnerLock & lock, uintptr_t info, uTime time );
	bool signal();
	bool broadcast();

	bool empty() const {
		return waiting.empty();
	} // uCondLock::empty

	uintptr_t front() const;

	void * operator new( size_t size ) {
		return ::operator new( size );
	} // Pthread_cleanup::operator new

	void * operator new( size_t, void * storage ) {		// used in pthread_cond
		return storage;
	} // uCondLock::operator new
}; // uCondLock


//######################### uSemaphore #########################


namespace UPP {
	class uSemaphore {
		struct TimedWaitHandler : public uSignalHandler { // real-time
			uSemaphore & semaphore;
			bool timedout;

			TimedWaitHandler( uBaseTask & task, uSemaphore & semaphore );
			TimedWaitHandler( uSemaphore & semaphore );
			void handler();
		}; // TimedWaitHandler

		// These data fields must be initialized to zero. Therefore, this lock can be used in the same storage area as a
		// sem_t, if sizeof(sem_t) >= sizeof(uSemaphore).

		uBaseSpinLock spinLock;							// must be first field for alignment
		int count;
		uQueue<uBaseTaskDL> waiting;

		void waitTimeout( TimedWaitHandler & h );
	  public:
		uSemaphore( const uSemaphore & ) = delete;		// no copy
		uSemaphore( uSemaphore && ) = delete;
		uSemaphore & operator=( const uSemaphore & ) = delete; // no assignment
		uSemaphore & operator=( uSemaphore && ) = delete;

		uSemaphore( int count = 1 ) : count( count ) {
#ifdef __U_STATISTICS__
			uFetchAdd( UPP::Statistics::uSemaphores, 1 );
#endif // __U_STATISTICS__
			uDEBUG(
				if ( count < 0 ) {
					abort( "Attempt to initialize uSemaphore %p to %d that must be >= 0.", this, count );
				} // if
			)
		} // uSemaphore::uSemaphore

		void P();										// semaphore wait
		void P( uintptr_t info );						// semaphore wait
		bool P( uDuration duration );					// semaphore wait or timeout
		bool P( uintptr_t info, uDuration duration );	// semaphore wait or timeout
		bool P( uTime time );							// semaphore wait or timeout
		bool P( uintptr_t info, uTime time );			// semaphore wait or timeout
		void P( uSemaphore & s );						// semaphore wait and release another
		void P( uSemaphore & s, uintptr_t info );		// semaphore wait and release another
		bool P( uSemaphore & s, uDuration duration );	// semaphore wait and release another or timeout
		bool P( uSemaphore & s, uintptr_t info, uDuration duration ); // semaphore wait and release another or timeout
		bool P( uSemaphore & s, uTime time );			// semaphore wait and release another or timeout
		bool P( uSemaphore & s, uintptr_t info, uTime time ); // semaphore wait and release another or timeout
		bool TryP();									// conditional semaphore wait
		void V();										// signal semaphore
		void V( int inc );								// signal semaphore

		uintptr_t front() const;						// return task information

		int counter() const {							// semaphore counter
			return count;
		} // uSemaphore::counter

		bool empty() const {							// no tasks waiting on semaphore ?
			return count >= 0;
		} // uSemaphore::empty

		void * operator new( size_t size ) {
			return ::operator new( size );
		} // uSemaphore::operator new

		void * operator new( size_t, void * storage ) {	// used in sem_t
			return storage;
		} // uSemaphore::operator new
	}; // uSemaphore
} // UPP


//######################### uContext #########################


class uContext : public uSeqable {
	void * key;
  public:
	uContext();
	uContext( void * key );
	virtual ~uContext();

	// These two routines cannot be abstract (i.e., = 0) because there is a race condition during initialization of a
	// derived class when the base class constructor is invoked. A context switch can occur immediately after the base
	// instance has put itself on the task context list but before the virtual function vector is updated for the
	// derived class.  Hence, the save or restore routine of the base class may be called.  This situtation is not a
	// problem because either the task has not performed any operations that involve the new context or the task is
	// removing the context and not performing anymore operations using it.

	virtual void save();
	virtual void restore();
}; // uContext

typedef uSequence<uContext> uContextSeq;


//######################### uFloatingPointContext #########################


// Provide floating point context switch support for coroutines and tasks.  It is built with the more general context
// class in the same way that a user can extend the amount of context that is saved and restored with a coroutine or
// task.

#if defined( __i386__ )
// saved by caller
#elif defined( __x86_64__ )
// saved by caller
#else
	#error uC++ : internal error, unsupported architecture
#endif


// Some architectures store the floating point registers with the integers registers during a basic context switch, so
// there is no need for a data area to storage the floating point registers.

#ifdef __U_FLOATINGPOINTDATASIZE__
class uFloatingPointContext : public uContext {
	static int uniqueKey;
	double floatingPointData[__U_FLOATINGPOINTDATASIZE__] __attribute__(( aligned(16) ));
  public:
	uFloatingPointContext();
#else
class uFloatingPointContext {
  public:
#endif // __U_FLOATINGPOINTDATASIZE__
	void save();										// save and restore the floating point context
	void restore();
} __attribute__(( unused )); // uFloatingPointContext


//######################### uMachContext #########################


extern "C" void uSwitch( void * from, void * to ) asm ("uSwitch"); // assembler routine that performs the context switch


// Contains the machine dependent context and routines that initialize and switch between contexts.

namespace UPP {
	class uMachContext {
		friend class ::uContext;						// access: extras, additionalContexts
		friend class ::uProcessorTask;					// access: size, base, limit
		friend class ::uBaseCoroutine;					// access: storage
		friend class ::uBaseTask;						// access: context
		friend _Coroutine uProcessorKernel;				// access: storage
		friend class ::uProcessor;						// access: storage
		friend class uKernelBoot;						// access: storage
		friend void * uKernelModule::startThread( void * p ); // acesss: invokeCoroutine

		struct uContext_t {								// name mimics ucontext_t from Linux headers
			void * SP, * FP;
		};

		static size_t pageSize;							// architecture pagesize
#if defined( __i386__ ) || defined( __x86_64__ )
		static uint16_t fncw;							// floating/MMX control registers
		static uint32_t mxcsr;
#endif // __i386__ || __x86_64__

		void * storage_;								// stack pointer
		void * limit_;									// stack grows towards stack limit
		void * base_;									// stack base
		void * context_;								// uContext_t pointer
		union {
			long int allExtras;							// allow access to all extra flags
			struct {									// put all extra flags in this structure
				unsigned int usercxts : 1;				// user defined contexts
			} is;
		} extras_;										// indicates extra work during the context switch

		void createContext( unsigned int stackSize );	// used by all constructors

		void startHere( void (* uInvoke)( uMachContext & ) );
	  protected:
		static void invokeCoroutine( uBaseCoroutine & This );
		static void invokeTask( uBaseTask & This ) __attribute__(( noreturn ));
		static void cleanup( uBaseTask & This ) __attribute__(( noreturn ));

		uContextSeq additionalContexts_;				// list of additional contexts for this execution state

		void extraSave();
		void extraRestore();

		void save() {
			// Any extra work that must occur on this side of a context switch is performed here.
			if ( UNLIKELY( extras_.allExtras ) ) {
				extraSave();
			} // if

			uDEBUG( verify(); )
		} // uMachContext::save

		void restore() {
			uDEBUG( verify(); )

			// Any extra work that must occur on this side of a context switch is performed here.
			if ( UNLIKELY( extras_.allExtras ) ) {
				extraRestore();
			} // if
		} // uMachContext::restore

		virtual void main() = 0;						// starting routine for coroutine or task
	  public:
		uMachContext( const uMachContext & ) = delete;	// no copy
		uMachContext( uMachContext && ) = delete;
		uMachContext & operator=( const uMachContext & ) = delete; // no assignment
		uMachContext & operator=( uMachContext && ) = delete;

		uMachContext( unsigned int stackSize ) {
			// stack storage provides a minimum of stackSize memory for the stack plus ancillary storage
			storage_ = nullptr;
			createContext( stackSize );
		} // uMachContext::uMachContext

		uMachContext( void * storage, unsigned int storageSize ) {
			// stack storage provides a maximum of memory for the stack plus ancillary storage
			storage_ = storage;
			createContext( storageSize );
		} // uMachContext::uMachContext

		virtual ~uMachContext() noexcept(false) {		// noexcept(false) inherited by subclass destructors
			if ( ! ((uintptr_t)storage_ & 1) ) {			// check user stack storage mark
				uDEBUG(
					if ( ::mprotect( storage_, pageSize, PROT_READ | PROT_WRITE ) == -1 ) {
						abort( "(uMachContext &)%p.~uMachContext() : internal error, mprotect failure, error(%d) %s.", this, errno, strerror( errno ) );
					} // if
				);
				free( storage_ );
			} // if
		} // uMachContext::~uMachContext

		void * stackPointer() const;

		unsigned int stackSize() const {
			return (char *)base_ - (char *)limit_;
		} // uMachContext::stackSize

		void * stackStorage() const {
			return (void *)((uintptr_t)storage_ & 1);	// remove user stack storage mark
		} // uMachContext::stackStorage

		ptrdiff_t stackFree() const;
		ptrdiff_t stackUsed() const;
		void verify();

		// These members should be private but cannot be because they are referenced from user code.

		static void * rtnAdr( void (* rtn)() );			// access: see profiler
	}; // uMachContext
} // UPP


//######################### uBaseCoroutine #########################


extern "C" void pthread_exit( void * status );

template< typename Actor > _Coroutine uCorActorType;	// see uActor.h

class uBaseCoroutine : public UPP::uMachContext {
	friend class UPP::uMachContext;						// access: notHalted, main, suspend, setState, corStarter, corFinish
	friend class uBaseTask;								// access: serial, profileTaskSamplerInstance
	friend class UPP::uKernelBoot;						// access: last
	friend _Task UPP::uBootTask;						// access: notHalted
	friend _Coroutine UPP::uProcessorKernel;			// access: taskCxtSw
	template< typename Actor > friend _Coroutine uCorActorType; // replace corFinish

	// cancellation

	friend _Task uPthreadable;							// access: Cleanup, PthreadCleanup

	// exception handling

	friend class uEHM;									// access: resumedObj, topResumedType, DEStack, handlerStackTop, handlerStackVisualTop, unexpectedRtn
	friend class uEHM::ResumeWorkHorseInit;				// access: resumedObj, topResumedType, handlerStackVisualTop
	friend class uEHM::uResumptionHandlers;				// access: handlerStackTop, handlerStackVisualTop
	friend class uEHM::uDeliverEStack;					// access: DEStack
	// Deprecated C++17
	friend std::unexpected_handler std::set_unexpected( std::unexpected_handler func ) throw(); // access: unexpectedRtn
	friend void uEHM::unexpected();						// access: unexpected

#ifdef __U_PROFILER__
	// profiling

	friend class uProfilerBoot;							// access: serial
	friend class uProfileTaskSampler;					// access: profileTaskSamplerInstance
#endif // __U_PROFILER__
  public:
	enum State { Start, Inactive, Active, Halt };
	enum CancellationState { CancelEnabled = PTHREAD_CANCEL_ENABLE, CancelDisabled = PTHREAD_CANCEL_DISABLE };
	enum CancellationType { CancelPoll = PTHREAD_CANCEL_DEFERRED, CancelImplicit = PTHREAD_CANCEL_ASYNCHRONOUS };
  private:
	const char * name_;									// textual name for coroutine/task, initialized by uC++ generated code
	uBaseCoroutine * starter_;							// first coroutine to resume this one
	UPP::uSerial * serial_;								// original serial instance for cormonitor/task (versus currently used instance)
	State state_;										// current execution status for coroutine
	bool notHalted_;									// indicate if execuation state is not halted

	uBaseCoroutine * last_;								// last coroutine to resume this one
	uBaseTask * currSerialOwner_;						// task accessing monitors from this coroutine
	unsigned int currSerialCount_;						// counter to determine when to unset currSerialOwner

	// cancellation

	_Event UnwindStack {
		friend class uBaseCoroutine;
		friend class UPP::uMachContext;					// access: exec_dtor
		friend _Task uPthread;							// access: exec_dtor

		bool exec_dtor;

		UnwindStack( bool = false );
	  public:
		~UnwindStack();						
	}; // uBaseCoroutine::UnwindStack

	bool cancelled_;									// cancellation flag
	bool cancelInProgress_;								// cancellation in progress flag
	CancellationState cancelState_;						// enabled/disabled
	CancellationType cancelType_;						// deferred/asynchronous

	struct PthreadCleanup : uColable {
		void (* routine )(void *);
		void * args;
		void * operator new( size_t, void * storage ) {
			return storage;
		} // Pthread_cleanup::operator new
	}; // PthreadCleanup

	typedef uStack<PthreadCleanup> Cleanup;

	void unwindStack();

	// exception handling

	uEHM::uResumptionHandlers * handlerStackTop_, * handlerStackVisualTop_;
	uBaseEvent * resumedObj_;							// the object that is currently being handled during resumption
	const std::type_info * topResumedType_;				// the top of the currently handled resumption stack (unchanged during stack unwind through EH)
	uEHM::uDeliverEStack * DEStack_;					// manage exception enable/disable
	std::unexpected_handler unexpectedRtn_;				// per coroutine handling unexpected action
	bool unexpected_;									// indicate if unexpected error occurs

	__cxxabiv1::__cxa_eh_globals ehGlobals;
	friend __cxxabiv1::__cxa_eh_globals *__cxxabiv1::__cxa_get_globals_fast() throw();
	friend __cxxabiv1::__cxa_eh_globals *__cxxabiv1::__cxa_get_globals() throw();

	// profiling : necessary for compatibility between non-profiling and profiling

	mutable uProfileTaskSampler * profileTaskSamplerInstance; // pointer to related profiling object

	void createCoroutine();

	void setState( State s ) {
		state_ = s;
	} // uBaseCoroutine::setState

	void taskCxtSw();									// switch between a task and the kernel
	void corCxtSw();									// switch between two coroutine contexts

	void corStarter() {									// remembers who started a coroutine
		starter_ = last_;
	} // uBaseCoroutine::corStarter

	virtual void corFinish() __attribute__(( noreturn ));
  public:
	// exception handling

	_Event Failure : public uKernelFailure {
	  protected:
		Failure( const char * const msg = "" );
	  public:
	}; // uBaseCoroutine::Failure

	_Event UnhandledException : public uBaseCoroutine::Failure {
		friend class uBaseCoroutine;					// access: all
		friend class uBaseTask;							// access: all

		uBaseEvent * cause;								// initial exception
		unsigned int multiple;							// multiple exceptions ?
		mutable bool cleanup;							// => delete "cause"
		UnhandledException( uBaseEvent * cause, const char * const msg = "" );
	  public:
		UnhandledException( const UnhandledException & ex );
		unsigned int unhandled() const { return multiple; }
		virtual ~UnhandledException();
		virtual void defaultTerminate() override;
		void triggerCause();
	}; // uBaseCoroutine::UnhandledException
  protected:
	// Duplicate "main" (see uMachContext) to get better error message about missing "main" member for coroutine.
	virtual void main() = 0;							// starting routine for coroutine

	// Only allow direct access to resume/suspend, i.e., preclude indirect access C.resume()/C.suspend()
	void resume() {										// restarts the coroutine's main where last suspended
		uBaseCoroutine & c = uThisCoroutine();			// optimization

		if ( & c != this ) {								// not resuming self ?
			uDEBUG(
				if ( ! notHalted_ ) {					// check if terminated
					abort( "Attempt by coroutine %.256s (%p) to resume terminated coroutine %.256s (%p).\n"
						   "Possible cause is terminated coroutine's main routine has already returned.",
						   c.getName(), &c, getName(), this );
				} // if
			);
			last_ = &c;									// set last resumer
		} // if
		corCxtSw();										// always done for performance testing

		_Enable <Failure>;								// implicit poll
	} // uBaseCoroutine::resume

	void suspend() {									// restarts the coroutine that most recently resumed this coroutine
		uBaseCoroutine & c = uThisCoroutine();			// optimization
			uDEBUG(
				if ( c.last_ == nullptr ) {
					abort( "Attempt to suspend coroutine %.256s (%p) that has never been resumed.\n"
						   "Possible cause is a suspend executed in a member called by a coroutine user rather than by the coroutine main.",
						   getName(), this );
				} // if
				if ( ! c.last_->notHalted_ ) {			// check if terminated
					abort( "Attempt by coroutine %.256s (%p) to suspend back to terminated coroutine %.256s (%p).\n"
						   "Possible cause is terminated coroutine's main routine has already returned.",
						   getName(), this, c.last_->getName(), c.last_ );
				} // if
			);
		c.last_->corCxtSw();

		_Enable <Failure>;								// implicit poll
	} // uBaseCoroutine::suspend

	class uCoroutineConstructor {						// placed in the constructor of a coroutine
	  public:
			uCoroutineConstructor( UPP::uAction f, UPP::uSerial & serial, uBaseCoroutine & coroutine, const char * name );
	} __attribute__(( unused )); // uCoroutineConstructor

	class uCoroutineDestructor {						// placed in the destructor of a coroutine
#ifdef __U_PROFILER__
		UPP::uAction f;
		uBaseCoroutine & coroutine;
#endif // __U_PROFILER__
	  public:
		uCoroutineDestructor(
#ifdef __U_PROFILER__
			UPP::uAction f,
#endif // __U_PROFILER__
			uBaseCoroutine & coroutine
		);
#ifdef __U_PROFILER__
		~uCoroutineDestructor();
#endif // __U_PROFILER__
	}; // uCoroutineDestructor
  public:
	uBaseCoroutine( const uBaseCoroutine & ) = delete;	// no copy
	uBaseCoroutine( uBaseCoroutine && ) = delete;
	uBaseCoroutine & operator=( const uBaseCoroutine & ) = delete; // no assignment
	uBaseCoroutine & operator=( uBaseCoroutine && ) = delete;

	uBaseCoroutine();

	uBaseCoroutine( unsigned int stackSize ) : UPP::uMachContext( stackSize ) {
		createCoroutine();
	} // uBaseCoroutine::uBaseCoroutine

	uBaseCoroutine( void * storage, unsigned int storageSize ) : UPP::uMachContext( storage, storageSize ) {
		createCoroutine();
	} // uBaseCoroutine::uBaseCoroutine

	const char * setName( const char * name );
	const char * getName() const;

	State getState() const {
		return notHalted_ ? state_ : Halt;
	} // uBaseCoroutine::getState

	uBaseCoroutine & starter() const {					// starter coroutine => did first resume
		return * starter_;
	} // uBaseCoroutine::starter

	uBaseCoroutine & resumer() const {					// last resumer coroutine
		return * last_;
	} // uBaseCoroutine::resumer

	// asynchronous exceptions

	static int asyncpoll() __attribute__(( deprecated )) { return uEHM::poll(); }

	// cancellation

	void cancel() { cancelled_ = true; }
	bool cancelled() { return cancelled_; }
	bool cancelInProgress() { return cancelInProgress_; }
  private:
	void forwardUnhandled( UnhandledException & ex );
	void handleUnhandled( uBaseEvent * ex = nullptr );
  public:
	// These members should be private but cannot be because they are referenced from user code.

	void setCancelState( CancellationState state );
	CancellationState getCancelState() { return cancelState_; }
	void setCancelType( CancellationType type );
	CancellationType getCancelType() { return cancelType_; }

	template<CancellationState newState> class Cancel {	// RAII helper to enable/disable cancellation
		CancellationState prev;
	  public:
		Cancel() {
			uBaseCoroutine & coroutine = uThisCoroutine();
			prev = coroutine.getCancelState();
			coroutine.setCancelState( newState );
		} // Cancel::Cancel

		~Cancel() {
			uThisCoroutine().setCancelState( prev );
		} // Cancel::~Cancel
	}; // uBaseCoroutine::Cancel

	uEHM::AsyncEMsgBuffer asyncEBuf;					// list of pending nonlocal exceptions
}; // uBaseCoroutine


//######################### Real-Time (cont) #########################


class uBaseScheduleFriend {
  protected:
	virtual ~uBaseScheduleFriend() {}
	uBaseTask & getInheritTask( uBaseTask & task ) const;
	int getActivePriority( uBaseTask & task ) const;
	int getActivePriorityValue( uBaseTask & task ) const;
	int setActivePriority( uBaseTask & task1, int priority );
	int setActivePriority( uBaseTask & task1, uBaseTask & task2 );
	int getBasePriority( uBaseTask & task ) const;
	int setBasePriority( uBaseTask & task, int priority );
	int getActiveQueueValue( uBaseTask & task ) const;
	int setActiveQueue( uBaseTask & task1, int priority );
	int getBaseQueue( uBaseTask & task ) const;
	int setBaseQueue( uBaseTask & task, int priority );
	bool isEntryBlocked( uBaseTask & task ) const;
	bool checkHookConditions( uBaseTask & task1, uBaseTask & task2 ) const;
}; // uBaseScheduleFriend


template<typename Node> class uBaseSchedule : protected uBaseScheduleFriend {
  public:
	virtual bool empty() const = 0;
	virtual void add( Node * node ) = 0;
	virtual Node * drop() = 0;
	virtual void remove( uBaseTaskDL * node ) = 0;
	virtual void transfer( uBaseTaskSeq & from ) = 0;
	virtual bool checkPriority( Node & owner, Node & calling ) = 0;
	virtual void resetPriority( Node & owner, Node & calling ) = 0;
	virtual void addInitialize( uBaseTaskSeq & taskList ) = 0;
	virtual void removeInitialize( uBaseTaskSeq & taskList ) = 0;
	virtual void rescheduleTask( uBaseTaskDL * taskNode, uBaseTaskSeq & taskList ) = 0;
}; // uBaseSchedule


class uBasePrioritySeq : public uBaseScheduleFriend {
	friend class UPP::uSerial;
  protected:
	uBaseTaskSeq list;
	bool executeHooks;
  public:
	uBasePrioritySeq() {
		executeHooks = false;
	} // uBasePrioritySeq::uBasePrioritySeq

	virtual bool empty() const {
		return list.empty();
	} // uBasePrioritySeq::empty

	virtual uBaseTaskDL * head() const {
		return list.head();
	} // uBasePrioritySeq::head

	virtual int add( uBaseTaskDL * node, uBaseTask * /* uOwner */ ) {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::mutex_queue, 1 );
#endif // __U_STATISTICS__
		list.addTail( node );
		return 0;
	} // uBasePrioritySeq::add

	virtual uBaseTaskDL * drop() {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::mutex_queue, -1 );
#endif // __U_STATISTICS__
		return list.dropHead();
	} // uBasePrioritySeq::drop

	virtual void remove( uBaseTaskDL * node ) {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::mutex_queue, -1 );
#endif // __U_STATISTICS__
		list.remove( node );
	} // uBasePrioritySeq::remove

	virtual void transfer( uBaseTaskSeq & /* from */ ) {
	} // uBasePrioritySeq::transfer

	virtual void onAcquire( uBaseTask & /* uOwner */ ) {
	} // uBasePrioritySeq::onAcquire

	virtual void onRelease( uBaseTask & /* uOldOwner */ ) {
	} // uBasePrioritySeq::onRelease

	int reposition( uBaseTask & task, UPP::uSerial & sserial );
}; // uBasePrioritySeq


class uBasePriorityQueue : public uBasePrioritySeq {
	uQueue<uBaseTaskDL> list;
  public:
	virtual bool empty() const {
		return list.empty();
	} // uBasePriorityQueue::empty

	virtual uBaseTaskDL * head() const {
		return list.head();
	} // uBasePriorityQueue::head

	virtual int add( uBaseTaskDL * node, uBaseTask * /* uOwner */ ) {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::mutex_queue, 1 );
#endif // __U_STATISTICS__
		list.add( node );
		return 0;										// dummy value
	} // uBasePriorityQueue::add

	virtual uBaseTaskDL * drop() {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::mutex_queue, -1 );
#endif // __U_STATISTICS__
		return list.drop();
	} // uBasePriorityQueue::drop

	virtual void remove( uBaseTaskDL * /* node */ ) {
		// Only used with default FIFO case, so node to remove is at the front of the list.
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::mutex_queue, -1 );
#endif // __U_STATISTICS__
		list.drop();
	} // uBasePriorityQueue::remove

	virtual void transfer( uBaseTaskSeq & /* from */ ) {
	} // uBasePriorityQueue::transfer

	virtual void onAcquire( uBaseTask & /* uOwner */ ) {
	} // uBasePriorityQueue::onAcquire

	virtual void onRelease( uBaseTask & /* uOldOwner */ ) {
	} // uBasePriorityQueue::onRelease
}; // uBasePriorityQueue


class uRepositionEntry {
	uBaseTask & blocked;
	UPP::uSerial & bSerial, & cSerial;
  public:
	uRepositionEntry( uBaseTask & blocked, uBaseTask & calling );
	int uReposition( bool relCallingLock );
}; // uRepositionEntry


class uDefaultScheduler : public uBaseSchedule<uBaseTaskDL> {
	uBaseTaskSeq list;									// list of tasks awaiting execution
  public:
	bool empty() const { return list.empty(); }
#ifdef KNOT
	void add( uBaseTaskDL * taskNode );
#else
	void add( uBaseTaskDL * taskNode ) { list.addTail( taskNode );
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::ready_queue, 1 );
#endif // __U_STATISTICS__
	}
#endif // KNOT

	uBaseTaskDL * drop() {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::ready_queue, -1 );
#endif // __U_STATISTICS__
		return list.dropHead();
	} // uDefaultScheduler::drop

	void remove( uBaseTaskDL * node ) {
#ifdef __U_STATISTICS__
		uFetchAdd( UPP::Statistics::ready_queue, -1 );
#endif // __U_STATISTICS__
		list.remove( node );
	} // uDefaultScheduler::remove

	void transfer( uBaseTaskSeq & from ) {
		list.transfer( from );
	} // uDefaultScheduler::remove

	bool checkPriority( uBaseTaskDL & /* owner */, uBaseTaskDL & /* calling */ ) { return false; }
	void resetPriority( uBaseTaskDL & /* owner */, uBaseTaskDL & /* calling */ ) {}
	void addInitialize( uBaseTaskSeq & /* taskList */ ) {};
	void removeInitialize( uBaseTaskSeq & /* taskList */ ) {};
	void rescheduleTask( uBaseTaskDL * /* taskNode */, uBaseTaskSeq & /* taskList */ ) {};
}; // uDefaultScheduler


//######################### Translator Generated Definitions #########################


class uBasePIQ {
  protected:
	virtual ~uBasePIQ() {}
  public:
	virtual int getHighestPriority() = 0;
}; // uBasePIQ


#include <uPIHeap.h>
#ifdef KNOT
#include <uCeilingQ.h>
#endif // KNOT


//######################### uBaseTask (cont) #########################


class uBaseTask : public uBaseCoroutine {
	friend class UPP::uSerial;							// access: everything
	friend class UPP::uSerialConstructor;				// access: profileActive, setSerial
	friend class UPP::uSerialDestructor;				// access: mutexRef_, profileActive, mutexRecursion, setState
	friend class UPP::uMachContext;						// access: currCoroutine_, profileActive, setState, main
//	friend class uTaskDestructor;						// cause
	friend class uBaseCoroutine;						// access: currCoroutine_, profileActive, setState
	friend uBaseCoroutine & uThisCoroutine();			// access: currCoroutine_
	friend class uMutexLock;							// access: entryRef_, profileActive, wake
	friend class uOwnerLock;							// access: entryRef_, profileActive, wake
	template< int, int, int > friend class uAdaptiveLock; // access: entryRef_, profileActive, wake
	friend class uCondLock;								// access: entryRef_, ownerLock, profileActive, wake
	friend class uBaseSpinLock;							// access: profileActive
	friend class UPP::uSemaphore;						// access: entryRef_, wake, info
	friend class uRWLock;								// access: entryRef_, wake, info
	friend class uCondition;							// access: currCoroutine_, mutexRef_, info, profileActive
	friend _Coroutine UPP::uProcessorKernel;			// access: currCoroutine_, setState, wake
	friend _Task uProcessorTask;						// access: currCluster, uBaseTask
	friend class uCluster;								// access: currCluster, readyRef_, clusterRef_, bound_
	friend _Task UPP::uBootTask;						// access: wake
	friend class UPP::uHeapManager;						// access: profileActive
	friend class uKernelModule;							// access: currCoroutine_, inheritTask
	friend void * malloc( size_t size ) __THROW;			// access: profileActive
	friend void * memalign( size_t alignment, size_t size ) __THROW; // access: profileActive
	friend class uEventList;							// access: profileActive
	friend class UPP::uHeapControl;						// access: heapData
	friend class uEventListPop;							// access: currCluster
#ifdef KNOT
	friend int pthread_mutex_lock( pthread_mutex_t * mutex ) __THROW; // access: setActivePriority
	friend int pthread_mutex_trylock( pthread_mutex_t * mutex ) __THROW; // access: setActivePriority
	friend int pthread_mutex_unlock( pthread_mutex_t * mutex ) __THROW; // access: setActivePriority
#endif // KNOT

	// exception handling

	friend void uEHM::terminateHandler();				// access: terminateRtn
	friend void std::terminate() noexcept;				// access: terminateRtn
	friend std::terminate_handler std::set_terminate( std::terminate_handler func ) noexcept; // access: terminateRtn

	// debugging

	friend class uLocalDebuggerHandler;					// access: taskDebugMask, processBP
	friend _Task uLocalDebugger;						// access: bound_, taskDebugMask, debugPCandSRR
	friend class UPP::uSigHandlerModule;				// access: debugPCandSRR

#ifdef __U_PROFILER__
	// profiling

	friend _Task uProfiler;								// access: currCoroutine_
	friend void __cyg_profile_func_enter( void * pcCurrentFunction, void * pcCallingFunction );
	friend void __cyg_profile_func_exit( void * pcCurrentFunction, void * pcCallingFunction );
	friend class uExecutionMonitor;						// access: profileActive
	friend void UPP::umainProfile();					// access: profileActive
#endif // __U_PROFILER__
  public:
	enum State { Start, Ready, Running, Blocked, Terminate };
  private:
	void createTask( uCluster & cluster );
	uBaseTask( uCluster & cluster, uProcessor & processor ); // only used by uProcessorTask
	void setState( State state );
	void wake();

	// debugging : must be first fields

	char taskDebugMask[8];								// 64 bit breakpoint mask for task (used only by debugger)
	void * debugPCandSRR;								// PC of break point; address of return message for IPC SRR
	bool processBP;										// true if task is in the middle of processing a breakpoint

	// general

	State state_;										// current state of task
	unsigned int recursion_;							// allow recursive entry of main member
	unsigned int mutexRecursion_;						// number of recursive calls while holding mutex
	uCluster * currCluster_;							// cluster task is executing on
	uBaseCoroutine * currCoroutine_;					// coroutine being executed by tasks thread
	uintptr_t info_;									// condition information stored with blocked task

	uBaseTaskDL clusterRef_;							// double link field: list of tasks on cluster
	uBaseTaskDL readyRef_;								// double link field: ready queue
	uBaseTaskDL entryRef_;								// double link field: general entry deque (all waiting tasks)
	uBaseTaskDL mutexRef_;								// double link field: mutex member, suspend stack, condition variable
	uProcessor & bound_;								// processor to which this task is bound, if applicable
	uBasePrioritySeq * calledEntryMem_;					// pointer to called mutex queue
	uMutexLock * ownerLock_;							// pointer to owner lock used for signalling conditions

	// profiling : necessary for compatibility between non-profiling and profiling

	bool profileActive;									// indicates if this context is supposed to be profiled
#ifdef __U_PROFILER__
	void profileActivate( uBaseTask & task );
#endif // __U_PROFILER__

	// exception handling

	UPP::uSerialMember * acceptedCall;					// pointer to the last mutex entry accepted by this thread
	std::terminate_handler terminateRtn __attribute__(( noreturn )); // per task handling termination action
	uBaseCoroutine::UnhandledException * cause;			// forwarded unhandled exception
  protected:
	// real-time

	friend class UPP::uSerialMember;					// access: setSerial, currCoroutine_, profileActive, acceptedCall
	friend class uBaseScheduleFriend;					// access: entryRef_, getInheritTask, setActivePriority, setBasePriority, setActiveQueue, setBaseQueue, uIsEntryBlocked
	friend class uWakeupHndlr;							// access: wake
	friend class uBasePrioritySeq;						// access: entryRef_, mutexRef_, calledEntryMem
	friend class uRepositionEntry;						// access: entryList TEMPORARY

	// Duplicate "main" (see uMachContext) to get better error message about
	// missing "main" member for task.
	virtual void main() = 0;							// starting routine for task

	int priority;
	int activePriority;
	uBaseTask * inheritTask;
	int queueIndex;
	int activeQueueIndex;
	UPP::uSerial * currSerial;							// current serial task is using (not original serial)

	unsigned int currSerialLevel;						// counter for checking non-nested entry/exit from multiple accessed mutex objects

	uBaseTask & getInheritTask() {
		return * inheritTask;
	} // uBaseTask::getInheritTask

	int setActivePriority( int priority ) {
		int temp = activePriority;
		activePriority = priority;
		return temp;
	} // uBaseTask::setActivePriority

	int setActivePriority( uBaseTask & task ) {
		int temp = activePriority;
		inheritTask = & task;
		activePriority = inheritTask->getActivePriority();
		return temp;
	} // uBaseTask::setActivePriority

	int setBasePriority( int priority ) {
		int temp = priority;
		uBaseTask::priority = priority;
		return temp;
	} // uBaseTask::setBasePriority

	int setActiveQueue( int q ) {
		int temp = activeQueueIndex;
		// uInheritTask = & t;  is this needed or should this just be called from setActivePriority ??
		activeQueueIndex = q;
		return temp;
	} // uBaseTask::setActiveQueue

	int setBaseQueue( int q ) {
		int temp = queueIndex;
		queueIndex = q;
		return temp;
	} // uBaseTask::setBaseQueue

	UPP::uSerial & setSerial( UPP::uSerial & serial ) {
		UPP::uSerial * temp = currSerial;
		currSerial = & serial;
		return * temp;
	} // uBaseTask::setSerial


	class uTaskConstructor {							// placed in the constructor of a task
		UPP::uAction f;
		UPP::uSerial & serial;
		uBaseTask & task;
	  public:
		uTaskConstructor( UPP::uAction f, UPP::uSerial & serial, uBaseTask & task, uBasePIQ & piq, const char * n, bool profile );
		~uTaskConstructor();
	} __attribute__(( unused )); // uTaskConstructor

	class uTaskDestructor {								// placed in the destructor of a task
		friend class uTaskConstructor;					// access: cleanup

		UPP::uAction f;
		uBaseTask & task;

		static void cleanup( uBaseTask & task );
	  public:
		uTaskDestructor( UPP::uAction f, uBaseTask & task ) : f( f ), task( task ) {
		} // uTaskDestructor::uTaskDestructor

		~uTaskDestructor() noexcept(false);
	}; // uTaskDestructor

	class uTaskMain {									// placed in the main member of a task
		uBaseTask & task;
	  public:
		uTaskMain( uBaseTask & task );
		~uTaskMain();
	}; // uTaskMain

	void forwardUnhandled( UnhandledException & ex );
	void handleUnhandled( uBaseEvent * ex = nullptr );
  public:
	uBaseTask( const uBaseTask & ) = delete;			// no copy
	uBaseTask( uBaseTask && ) = delete;
	uBaseTask & operator=( const uBaseTask & ) = delete; // no assignment
	uBaseTask & operator=( uBaseTask && ) = delete;

	uBaseTask() : clusterRef_( * this ), readyRef_( * this ), entryRef_( * this ), mutexRef_( * this ), bound_( *(uProcessor *)0 ) {
		createTask( uThisCluster() );
	} // uBaseTask::uBaseTask

	uBaseTask( unsigned int stackSize ) : uBaseCoroutine ( stackSize ), clusterRef_( * this ), readyRef_( * this ), entryRef_( * this ), mutexRef_( * this ), bound_( *(uProcessor *)0 ) {
		createTask( uThisCluster() );
	} // uBaseTask::uBaseTask

	uBaseTask( void * storage, unsigned int storageSize ) : uBaseCoroutine ( storage, storageSize ), clusterRef_( * this ), readyRef_( * this ), entryRef_( * this ), mutexRef_( * this ), bound_( *(uProcessor *)0 ) {
		createTask( uThisCluster() );
	} // uBaseTask::uBaseTask

	uBaseTask( uCluster & cluster );

	uBaseTask( uCluster & cluster, unsigned int stackSize ) : uBaseCoroutine( stackSize ), clusterRef_( * this ), readyRef_( * this ), entryRef_( * this ), mutexRef_( * this ), bound_( *(uProcessor *)0 ) {
		createTask( cluster );
	} // uBaseTask::uBaseTask

	uBaseTask( uCluster & cluster, void * storage, unsigned int storageSize ) : uBaseCoroutine( storage, storageSize ), clusterRef_( * this ), readyRef_( * this ), entryRef_( * this ), mutexRef_( * this ), bound_( *(uProcessor *)0 ) {
		createTask( cluster );
	} // uBaseTask::uBaseTask

	~uBaseTask() {
	} // uBaseTask::~uBaseTask

	static void yield() {
		uThisTask().uYieldNoPoll();
		uEHM::poll();
	} // uBaseTask::yield

	static void yield( unsigned int times ) {
		for ( ; times > 0 ; times -= 1 ) {
			yield();
		} // for
	} // uBaseTask::yield

	static void sleep( uDuration duration );
	static void sleep( uTime time );

	static uCluster & migrate( uCluster & cluster );

	uCluster & getCluster() const {
		return * currCluster_;
	} // uBaseTask::getCluster

	uBaseCoroutine & getCoroutine() const {
		return * currCoroutine_;
	} // uBaseTask::getCoroutine

	State getState() const {
		return state_;
	} // uBaseTask::getState

	int getActivePriority() const {
		// special case for base of active priority stack
		return this == inheritTask ? priority : activePriority;
	} // uBaseTask::getActivePriority

	int getActivePriorityValue() const {				// TEMPORARY: replace previous member?
		 return activePriority;
	} // uBaseTask::getActivePriorityValue

	int getBasePriority() const {
		return priority;
	} // uBaseTask::getBasePriority

	int getActiveQueueValue() const {					// TEMPORARY: rename
		return activeQueueIndex;
	} // uBaseTask::getActiveQueueValue

	int getBaseQueue() const {
		return queueIndex;
	} // uBaseTask::getBaseQueue

	UPP::uSerial & getSerial() const {
		return * currSerial;
	} // uBaseTask::getSerial

#ifdef __U_PROFILER__
	// profiling

	void profileActivate();
	void profileInactivate();
	void printCallStack() const;
#endif // __U_PROFILER__

	// These members should be private but cannot be because they are referenced from user code.

	uBasePIQ * uPIQ;									// TEMPORARY
	void * pthreadData;									// pointer to pthread specific data
	void * heapData;									// thread-local storage for per-thread heaps

	void uYieldNoPoll();
	void uYieldYield( unsigned int times );				// inserted by translator for -yield
	void uYieldInvoluntary();							// pre-allocates metric memory before yielding
}; // uBaseTask


class uWakeupHndlr : public uSignalHandler {			// real-time
	friend class uBaseTask;								// access: uWakeupHndlr

	uWakeupHndlr( uBaseTask & task ) {
		This = & task;
	} // uWakeupHndlr::uWakeupHndlr

	void handler() {
		This->wake();
	} // uWakeupHndlr::handler
}; // uWakeupHndlr


inline uBaseCoroutine & uThisCoroutine() {
	return * uThisTask().currCoroutine_;
} // uThisCoroutine


namespace UPP {
	class uSerial {
		friend class ::uCondition;						// access: acceptSignalled, leave2
		friend class uSerialConstructor;				// access: mr, prevSerial, leave
		friend class uSerialDestructor;					// access: prevSerial, acceptSignalled, lastAcceptor, mutexOwner, leave, ~uSerialDestructor, enterDestructor
		friend class uSerialMember;						// access: lastAcceptor, notAlive, enter, leave
		friend class uBaseTask::uTaskConstructor;		// access: acceptSignalled
		friend class uMachContext;						// access: leave2
		friend _Task uBootTask;							// access: acceptSignalled
		friend class ::uBasePrioritySeq;				// access: mutexOwnerlock TEMPORARY
		friend class ::uRepositionEntry;				// access: lock, entryList TEMPORARY
		friend class ::uBaseScheduleFriend;				// access: checkHookConditions
		friend class ::uTimeoutHndlr;					// access: enterTimeout

#ifdef __U_PROFILER__
		// profiling

		friend class ::uProfileTaskSampler;				// access: profileTaskSamplerInstance
#endif // __U_PROFILER__

		// must be first field for alignment
		uSpinLock spinLock;								// provide mutual exclusion while examining serial state
		uBaseTask * mutexOwner;							// active thread in the mutex object
		uBitSet< __U_MAXENTRYBITS__ > mask;				// entry mask of accepted mutex members and timeout
		unsigned int * mutexMaskLocn;					// location to place mask position in accept statement
		uBasePrioritySeq & entryList;					// tasks waiting to enter mutex object
		uStack<uBaseTaskDL> acceptSignalled;			// tasks suspended within the mutex object
		uBaseTask * constructorTask;						// identity of task creating mutex object
		uBaseTask * destructorTask;						// identity of task calling mutex object's destructor
		uSerial * prevSerial;							// task's previous serial (see uSerialMember, recursive entry during constructor)
		unsigned int mr;								// mutex recursion counter for multiple serial-object entry
		enum uDestructorState { NoDestructor, DestrCalled, DestrScheduled }; // identify the state of the destructor
		uDestructorState destructorStatus;				// has the destructor been called ? 
		bool notAlive;									// serial destroyed ?
		bool acceptMask;								// entry mask set by uAcceptReturn or uAcceptWait
		bool acceptLocked;								// flag indicating if mutex lock has been acquired for the accept statement

		// real-time

		uEventNode timeoutEvent;						// event node for event list
		uEventList * events;								// event list when event added

		// exception handling

		uBaseTask * lastAcceptor;						// acceptor of current entry for communication between acceptor and caller

		// profiling : necessary for compatibility between non-profiling and profiling

		mutable uProfileTaskSampler * profileSerialSamplerInstance; // pointer to related profiling object

		void resetDestructorStatus();					// allow destructor to be called
		void enter( unsigned int & mr, uBasePrioritySeq & ml, int mp );
		void enterDestructor( unsigned int & mr, uBasePrioritySeq & ml, int mp );
		void enterTimeout();
		void leave( unsigned int mr );
		void leave2();
		void removeTimeout();
		bool checkHookConditions( uBaseTask * task );	// check conditions for executing hooks

		void acceptStart( unsigned int & mutexMaskPosn );
		void acceptTry();

		bool acceptTestMask() {
			return mask.isAllClr();
		} // uSerial::acceptTestMask

		void acceptPause();
		void acceptPause( uDuration duration );
		void acceptPause( uTime time );
		void acceptEnd();

		void acceptElse() {
			if ( acceptLocked ) {
				mask.clrAll();
				spinLock.release();
			} // if
		} // uSerial::acceptElse


		class uTimeoutHndlr : public uSignalHandler {	// real-time
			friend class uSerial;						// access: uWakeupHndlr

			uSerial & serial;

			uTimeoutHndlr( uBaseTask & task, UPP::uSerial & serial ) : serial( serial ) {
				This = & task;
			} // uTimeoutHndlr::uTimeoutHndlr

			uTimeoutHndlr( UPP::uSerial & serial ) : serial( serial ) {
				This = nullptr;
			} // uTimeoutHndlr::uTimeoutHndlr

			void handler() {
				serial.enterTimeout();
			} // uTimeoutHndlr::handler
		}; // uTimeoutHndlr
	  public:
		uSerial( const uSerial & ) = delete;			// no copy
		uSerial( uSerial && ) = delete;
		uSerial & operator=( const uSerial & ) = delete; // no assignment
		uSerial & operator=( uSerial && ) = delete;

		uSerial( uBasePrioritySeq & entryList );
		~uSerial();

		// These members should be private but cannot be because they are referenced from user code.

		// calls generated by translator in application code
		bool acceptTry( uBasePrioritySeq & ml, int mp );
		bool acceptTry2( uBasePrioritySeq & ml, int mp );

		void acceptSetMask() {
			// The lock acquired at the start of the accept statement cannot be released here, otherwise, it is
			// necessary to recheck the mutex queues before exit. As a consequence, all destructors between here and
			// ~uSerialMember (which executes leave) are executed with the mutex lock closed, preventing tasks from
			// queuing on this mutex object.
			acceptMask = true;
		} // uSerial::acceptSetMask

		bool executeU() {
			acceptPause();
			return true;
		} // uSerial::executeU

		bool executeC() {
			if ( acceptTestMask() ) {
				acceptElse();
				return false;
			} // if
			return executeU();
		} // uSerial::executeC

		bool executeU( bool else_ ) {
			if ( else_ ) {
				acceptElse();
				return false;
			} // if
			return executeU();
		} // uSerial::executeU

		bool executeC( bool else_ ) {
			if ( else_ ) {
				acceptElse();
				return false;
			} // if
			return executeC();
		} // uSerial::executeC

		bool executeU( bool timeout, uTime time ) {
			if ( timeout ) {
				acceptTry();
				acceptPause( time );
				return true;
			} // if
			return executeU();
		} // uSerial::executeU

		bool executeU( bool timeout, uDuration duration );

		bool executeC( bool timeout, uTime time ) {
			if ( timeout ) {
				acceptTry();
				acceptPause( time );
				return true;
			} // if
			return executeC();
		} // uSerial::executeC

		bool executeC( bool timeout, uDuration duration );

		bool executeU( bool timeout, uTime time, bool else_ ) {
			if ( else_ ) {
				acceptElse();
				return false;
			} // if
			if ( timeout ) {
				return executeU( timeout, time );
			} // if
			return executeU();
		} // uSerial::executeU

		bool executeU( bool timeout, uDuration duration, bool else_ );

		bool executeC( bool timeout, uTime time, bool else_ ) {
			if ( else_ ) {
				acceptElse();
				return false;
			} // if
			if ( timeout ) {
				return executeC( timeout, time );
			} // if
			return executeC();
		} // uSerial::executeC

		bool executeC( bool timeout, uDuration duration, bool else_ );

		class uProtectAcceptStmt {
			uSerial & serial;
		  public:
			unsigned int mutexMaskPosn;					// bit position (0-N) in the entry mask for accepted mutex member

			uProtectAcceptStmt( uSerial & serial ) : serial( serial ) {
				serial.acceptStart( mutexMaskPosn );
			} // uSerial::uProtectAcceptStmt::uProtectAcceptStmt

			uProtectAcceptStmt( uSerial & serial, bool ) : serial( serial ) {
				serial.removeTimeout();
				serial.acceptStart( mutexMaskPosn );
			} // uSerial::uProtectAcceptStmt::uProtectAcceptStmt

			~uProtectAcceptStmt() {
				serial.acceptEnd();
			} // uSerial::uProtectAcceptStmt::~uProtectAcceptStmt
		}; // uSerial::uProtectAcceptStmt
	}; // uSerial


	class uSerialConstructor {							// placed in the constructor of a mutex class
		uAction f;
		UPP::uSerial & serial;
	  public:
		uSerialConstructor( uAction f, uSerial & serial );
#ifdef __U_PROFILER__
		uSerialConstructor( uAction f, uSerial & serial, const char * n );
#endif // __U_PROFILER__
		~uSerialConstructor();
	}; // uSerialConstructor


	class uSerialDestructor {							// placed in the destructor of a mutex class
		uDEBUG(
			unsigned int nlevel;						// nesting level counter for accessed serial-objects
		)
		unsigned int mr;								// mutex recursion counter for multiple serial-object entry
		uAction f;
	  public:
		uSerialDestructor( uAction f, uSerial & serial, uBasePrioritySeq & ml, int mp );
		~uSerialDestructor();
	}; // uSerialDestructor


	class uSerialMember {								// placed in the mutex member of a mutex class
		uSerial * prevSerial;							// task's previous serial
		unsigned int mr;								// mutex recursion counter for multiple serial-object entry
		uDEBUG(
			unsigned int nlevel;						// nesting level counter for accessed serial-objects
		)

		// exception handling

		friend class uSerial;							// access: caller, acceptor
		friend class ::uCondition;						// access: caller

		uBaseTask * acceptor;							// acceptor of the entry invocation, null => no acceptor
		bool acceptorSuspended;							// true only when the acceptor, if there is one, remains blocked
		bool noUserOverride;							// true when acceptor has not been called

		void finalize( uBaseTask & task );
	  public:
		uSerialMember( uSerial & serial, uBasePrioritySeq & ml, int mp );
		~uSerialMember();
		uBaseTask * uAcceptor();
	}; // uSerialMember
} // UPP


//######################### uEHM (cont) #########################


inline bool uEHM::pollCheck() {
	uBaseCoroutine & coroutine = uThisCoroutine();
	return ! coroutine.asyncEBuf.empty() || coroutine.cancelled();
} // uEHM::pollCheck 


//######################### Cancellation #########################


class uEnableCancel : public uBaseCoroutine::Cancel<uBaseCoroutine::CancelEnabled> {
  public:
	uEnableCancel() {
		uEHM::poll();
	} // uEnableCancel::uEnableCancel
}; // uEnableCancel

class uDisableCancel : public uBaseCoroutine::Cancel<uBaseCoroutine::CancelDisabled> {
}; // uDisableCancel


//######################### uCondition #########################


class uCondition {
	uQueue<uBaseTaskDL> waiting;						// queue of blocked tasks
	UPP::uSerial * owner;								// mutex object owning condition, only set in wait
  public:
	uCondition( const uCondition & ) = delete;			// no copy
	uCondition( uCondition && ) = delete;
	uCondition & operator=( const uCondition & ) = delete; // no assignment
	uCondition & operator=( uCondition && ) = delete;

	uCondition() : owner( nullptr ) {
	} // uCondition::uCondition

	~uCondition();

	void wait();										// wait on condition
	void wait( uintptr_t info ) {						// wait on a condition with information
		uThisTask().info_ = info;						// store the information with this task
		wait();											// wait on this condition
	} // uCondition::wait
	bool signal();										// signal condition
	bool signalBlock();									// signal condition

	bool empty() const {								// test for tasks on a condition
		return waiting.empty();							// check if the condition queue is empty
	} // uCondition::empty

	uintptr_t front() const {							// return task information
		uDEBUG(
			if ( waiting.empty() ) {					// condition queue must not be empty
				abort( "Attempt to access user data on an empty condition.\n"
					   "Possible cause is not checking if the condition is empty before reading stored data." );
			} // if
		)
		return waiting.head()->task().info_;			// return condition information stored with blocked task
	} // uCondition::front

	// exception handling

	_Event WaitingFailure : public uKernelFailure {		// condition queue deleted before restarted from waiting
		friend class uCondition;

		const uCondition & cond;
		WaitingFailure( const uCondition & cond, const char * const msg = "" );
	  public:
		virtual ~WaitingFailure();
		const uCondition & conditionId() const;
		virtual void defaultTerminate() override;
	}; // uCondition::WaitingFailure 
}; // uCondition


//######################### uNBIO #########################


namespace UPP {
#ifdef KNOT
	_Mutex<uCeilingQ,uCeilingQ> class uNBIO {
#else
	class uNBIO {										// monitor (private mutex member)
#endif
		friend class ::uCluster;						// access: NBIO
		friend _Coroutine uProcessorKernel;				// access: okToSelect, IOPoller
		friend class uSelectTimeoutHndlr;				// access: NBIOnode
		friend class uKernelBoot;						// access: uNBIO

		struct NBIOnode : public uSeqable {
			uSemaphore pending;							// wait for I/O completion
			uBaseTask * pendingTask;					// name of waiting task in case nominated to IOPoller
			int nfds;									// return value
			enum { singleFd, multipleFds } fdType;
			bool timedout;								// has timeout
			bool * nbioTimeout;							// timeout in NBIO
			union {
				struct {								// used if waiting for only one fd
					uIOClosure * closure;
					int * uRWE;
				} sfd;
				struct {								// used if waiting for multiple fds
					unsigned int tnfds;
					fd_set * trfds;
					fd_set * twfds;
					fd_set * tefds;
				} mfd;
			} smfd;
		}; // NBIOnode

		class uSelectTimeoutHndlr : public uSignalHandler { // real-time
			NBIOnode & node;
			uCluster & cluster;
		  public:
			uSelectTimeoutHndlr( uBaseTask & task, NBIOnode & node ) : node( node ), cluster( uThisCluster() ) {
				This = & task;
			} // uSelectTimeoutHndlr::uSelectTimeoutHndlr

			void handler();
		}; // uSelectTimeoutHndlr

		uSequence<NBIOnode> pendingIOSfds[FD_SETSIZE];	// array of lists containing tasks waiting for an I/O event on a specific FD
		uSequence<NBIOnode> pendingIOMfds;				// list of tasks waiting for an I/O event on a general FD mask or timeout

		fd_set mRFDs, mWFDs, mEFDs;						// master copy of all single and multiple I/O
		fd_set srfds, swfds, sefds;						// master copy of all single I/O
		fd_set mrfds, mwfds, mefds;						// master copy of all multiple I/O
		bool efdsUsed;									// optimize out efds set is never used

		unsigned int maxFD;								// highest FD used in combined master mask
		unsigned int smaxFD;							// highest FD used in single master mask
		unsigned int mmaxFD;							// highest FD used in multiple master mask
		int descriptors;								// declared here so uniprocessor kernel can check if I/O occurred
		uBaseTask * IOPoller;							// pointer to current IO poller task, or 0
		unsigned int pending;
		uPid_t IOPollerPid;								// processor where IOPoller select blocks
		bool selectBlock;								// true => select blocks rather than poll
		bool timeoutOccurred;							// set when a waiting task times out
#if ! defined( __U_MULTI__ )
		bool okToSelect;								// uniprocessor flag indicating blocking select
#endif // ! __U_MULTI__

		_Mutex void checkIOStart();
		bool pollIO( NBIOnode & node );
		void performIO( int fd, NBIOnode * p, uSequence<NBIOnode> & pendingIO, int cnt );
		void checkSfds( int fd, NBIOnode * p, uSequence<NBIOnode> & pendingIO );
		void unblockFD( uSequence<NBIOnode> & pendingIO );
		_Mutex bool checkIOEnd( NBIOnode & node, int terrno );
		bool checkPoller();
		void waitOrPoll( NBIOnode & node, uEventNode * timeoutEvent = nullptr );
		void waitOrPoll( unsigned int nfds, NBIOnode & node, uEventNode * timeoutEvent = nullptr );
		_Mutex bool initSfd( NBIOnode & node, uEventNode * timeoutEvent = nullptr );
		_Mutex bool initMfds( unsigned int nfds, NBIOnode & node, uEventNode * timeoutEvent = nullptr );
		int select( sigset_t * );
		int select( uIOClosure & closure, int & rwe, timeval * timeout = nullptr );
		int select( int nfds, fd_set * rfds, fd_set * wfds, fd_set * efds, timeval * timeout = nullptr );

		uNBIO();
	  public:
	}; // uNBIO
} // UPP


//######################### uProcessorKernel #########################


template< int, int, int > class uAdaptiveLock;

namespace UPP {
	_Coroutine uProcessorKernel {
		friend class uKernelBoot;						// access: new, uProcessorKernel, ~uProcessorKernel
		friend class uSerial;							// access: schedule
		friend class uSerialDestructor;					// access: schedule
		friend class ::uMutexLock;						// access: schedule
		friend class ::uOwnerLock;						// access: schedule
		template<int, int, int> friend class ::uAdaptiveLock; // access: entryRef_, profileActive, wake
		friend class ::uCondLock;						// access: schedule
		friend class uSemaphore;						// access: schedule
		friend class ::uRWLock;							// access: schedule
		friend class ::uBaseTask;						// access: schedule
		friend _Task ::uProcessorTask;					// access: terminated
		friend class ::uProcessor;						// access: uProcessorKernel
		friend class uNBIO;								// access: kernelClock

		// real-time

		friend class ::uEventList;						// access: schedule
		friend class ::uEventListPop;					// access: kernelClock

		unsigned int kind;								// specific kind of schedule operation
		uBaseSpinLock * prevLock;						// comunication
		uBaseTask * nextTask;							// task to be wakened

		void taskIsBlocking();
		static void schedule();
		static void schedule( uBaseSpinLock * lock );
		static void schedule( uBaseTask * task );
		static void schedule( uBaseSpinLock * lock, uBaseTask * task );
		void scheduleInternal();
		void scheduleInternal( uBaseSpinLock * lock );
		void scheduleInternal( uBaseTask * task );
		void scheduleInternal( uBaseSpinLock * lock, uBaseTask * task );
		void onBehalfOfUser();
		void setTimer( uDuration time );
		void setTimer( uTime time );
#ifndef __U_MULTI__
		void nextProcessor( uProcessorDL *& currProc, uProcessorDL * cycleStart );
#endif // __U_MULTI__
		void main();

		uProcessorKernel();
		~uProcessorKernel();

		void * operator new( size_t size ) {
			return ::operator new( size );
		} // uProcessorKernel::operator new
	  public:
	}; // uProcessorKernel
} // UPP


//######################### uBaseTask (cont) #########################


inline void uBaseTask::uYieldNoPoll() {
	uDEBUG( assert( ! THREAD_GETMEM( disableIntSpin ) ); )
	UPP::uProcessorKernel::schedule( this );			// find someone else to execute; wake on kernel stack
} // uBaseTask::uYieldNoPoll


//######################### uProcessor (cont) #########################


class uProcessor {
	friend class UPP::uKernelBoot;						// access: new, uProcessor, events, contextEvent, contextSwitchHandler, setContextSwitchEvent
	friend class UPP::uInitProcessorsBoot;
	friend class uKernelModule;							// access: events
	friend class uCluster;								// access: pid, idleRef, external, processorRef, setContextSwitchEvent
	friend _Coroutine UPP::uProcessorKernel;			// access: events, currCluster_, procTask, external, globalRef, setContextSwitchEvent
	friend _Task uProcessorTask;						// access: pid, processorClock, preemption, currCluster_, setContextSwitchEvent
	friend class UPP::uNBIO;							// access: setContextSwitchEvent
	friend class uEventList;							// access: events, contextSwitchHandler
	friend class uEventNode;							// access: events
	friend class uEventListPop;							// access: contextSwitchHandler
	friend void * uKernelModule::startThread( void * p ); // acesss: everything
	friend class UPP::uMachContext;						// access: procTask
//#if defined( __i386__ ) && ! defined( __old_perfmon__ )
//	friend class HWCounters;							// access: uPerfctrContext (i386) or uPerfmon_fd (ia64)
//#endif

	// debugging

	friend _Task uLocalDebugger;						// access: debugIgnore
	friend _Task uLocalDebuggerReader;					// access: debugIgnore

	bool debugIgnore;									// ignore processor migration

#ifdef __U_PROFILER__
	// profiling
	
	friend class uProfileProcessorSampler;				// access: profileProcessorSamplerInstance
#endif // __U_PROFILER__

	static uEventList * events;							// single list of events for all processors
#if ! defined( __U_MULTI__ )
	static												// shared info on uniprocessor
#endif // ! __U_MULTI__
	uEventNode * contextEvent;							// context-switch node for event list
#if ! defined( __U_MULTI__ )
	static												// shared info on uniprocessor
#endif // ! __U_MULTI__
	uCxtSwtchHndlr * contextSwitchHandler;				// special time slice handler

#ifdef __U_MULTI__
	UPP::uProcessorKernel processorKer;					// need a uProcessorKernel
#endif // __U_MULTI__

	// profiling : necessary for compatibility between non-profiling and profiling

#ifndef __U_MULTI__
	static
#else
	mutable
#endif // ! __U_MULTI__
	uProfileProcessorSampler * profileProcessorSamplerInstance; // pointer to related profiling object

	void * operator new( size_t, void * storage ) {
		return storage;
	} // uProcessor::operator new
  protected:
	uPid_t pid;

	unsigned int preemption;
	unsigned int spin;

	uProcessorTask * procTask;							// handle processor specific requests
	uBaseTaskSeq external;								// ready queue for processor task

	uCluster * currCluster_;								// cluster processor currently associated with

	bool detached;										// processor detached ?
	bool terminated;									// processor being deleted ?

	uProcessorDL idleRef;								// double link field: list of idle processors
	uProcessorDL processorRef;							// double link field: list of processors on a cluster
	uProcessorDL globalRef;								// double link field: list of all processors

	void createProcessor( uCluster & cluster, bool detached, int ms, int spin );
	void fork( uProcessor * processor );
	void setContextSwitchEvent( int msecs );			// set the real-time timer
	void setContextSwitchEvent( uDuration duration );	// set the real-time timer

	uProcessor( uCluster & cluster, double );			// used solely during kernel boot
  public:
	uProcessor( const uProcessor & ) = delete;			// no copy
	uProcessor( uProcessor && ) = delete;
	uProcessor & operator=( const uProcessor & ) = delete; // no assignment
	uProcessor & operator=( uProcessor && ) = delete;

	uProcessor( unsigned int ms = uDefaultPreemption(), unsigned int spin = uDefaultSpin() );
	uProcessor( bool detached, unsigned int ms = uDefaultPreemption(), unsigned int spin = uDefaultSpin() );
	uProcessor( uCluster & cluster, unsigned int ms = uDefaultPreemption(), unsigned int spin = uDefaultSpin() );
	uProcessor( uCluster & cluster, bool detached, unsigned int ms = uDefaultPreemption(), unsigned int spin = uDefaultSpin() );
	~uProcessor();

	uPid_t getPid() const {
		return pid;
	} // uProcessor::getPid

	uCluster & setCluster( uCluster & cluster );

	uCluster & getCluster() const {
		return * currCluster_;
	} // uProcessor::getCluster

	bool getDetach() const {
		return detached;
	} // uProcessor::getTask

	unsigned int setPreemption( unsigned int ms );

	unsigned int getPreemption() const {
		return preemption;
	} // uProcessor::getPreemption

	unsigned int setSpin( unsigned int spin ) {
		int prev = spin;
		uProcessor::spin = spin;
		return prev;
	} // uProcessor::setSpin

	unsigned int getSpin() const {
		return spin;
	} // uProcessor::getSpin

#if defined( __U_AFFINITY__ )
	void setAffinity( const cpu_set_t & mask );
	void setAffinity( unsigned int cpu );
	void getAffinity( cpu_set_t & mask);
	int getAffinity();
#endif // __U_AFFINITY__

	bool idle() const {
		return idleRef.listed();
	} // uProcessor::idle

	void * operator new( size_t size ) {
		return ::operator new( size );
	} // uProcessor::operator new
} __attribute__(( unused )); // uProcessor


//######################### uSerial (cont) #########################


namespace UPP {
	inline bool uSerial::executeU( bool timeout, uDuration duration ) {
		return executeU( timeout, uClock::currTime() + duration );
	} // uSerial::executeU

	inline bool uSerial::executeC( bool timeout, uDuration duration ) {
		return executeC( timeout, uClock::currTime() + duration );
	} // uSerial::executeC

	inline bool uSerial::executeU( bool timeout, uDuration duration, bool else_ ) {
		return executeU( timeout, uClock::currTime() + duration, else_ );
	} // uSerial::executeU

	inline bool uSerial::executeC( bool timeout, uDuration duration, bool else_ ) {
		return executeC( timeout, uClock::currTime() + duration, else_ );
	} // uSerial::executeC
} // UPP


//######################### uCluster (cont) #########################


class uCluster {
	// The cluster cannot be a monitor because the processor kernel is only a coroutine. If the kernel called a mutex
	// routine in the cluster and could not get in, the previous task executed by the kernel is put on the mutex entry
	// queue and the kernel is restarted. When the kernel restarts, it now enters the critical section when it should be
	// scheduling a new task. Therefore explicit locks must be used for these queues.

	friend class uBaseTask;								// access: makeTaskReady, taskAdd, taskRemove
	friend class UPP::uNBIO;							// access: makeProcessorIdle, makeProcessorActive
	friend class uEventListPop;							// access: processorsOnCluster
	friend class UPP::uNBIO::uSelectTimeoutHndlr;		// access: NBIO, wakeProcessor
	friend class UPP::uKernelBoot;						// access: new, NBIO, taskAdd, taskRemove
	friend _Coroutine UPP::uProcessorKernel;			// access: NBIO, readyQueueTryRemove, readyQueueEmpty, tasksOnCluster, makeProcessorActive, processorPause
	friend _Task uProcessorTask;						// access: processorAdd, processorRemove
	friend class uProcessor;							// access: processorAdd, processorRemove
	friend class uRealTimeBaseTask;						// access: taskReschedule
	friend class uPeriodicBaseTask;						// access: taskReschedule
	friend class uSporadicBaseTask;						// access: taskReschedule
	friend struct uIOClosure;							// access: select
	friend class uRWLock;								// access: makeTaskReady

	// must be first field for alignment
	uSpinLock readyIdleTaskLock;						// protect readyQueue, idleProcessors and tasksOnCluster
	uSpinLock processorsOnClusterLock;

	// debugging

	friend _Task uLocalDebugger;						// access: debugIgnore
	friend _Task uLocalDebuggerReader;					// access: debugIgnore

	bool debugIgnore;									// ignore cluster migration

#ifdef __U_PROFILER__
	// profiling

	friend class uProfileClusterSampler;				// access: profileClusterSamplerInstance
#endif // __U_PROFILER__

	// real-time

	friend class uEventList;							// access: wakeProcessor
	friend class uProcWakeupHndlr;						// access: wakeProcessor

	uClusterDL globalRef;								// double link field: list of all clusters
  protected:
	void * operator new( size_t, void * storage ) {
		return storage;
	} // uCluster::operator new

	const char * name;									// textual name for cluster, default value
	uBaseSchedule<uBaseTaskDL> * readyQueue;				// list of tasks awaiting execution by processors on this cluster
	bool defaultReadyQueue;								// indicates if the cluster allocated the ready queue
	unsigned int idleProcessorsCnt;						// number of idle processors
	uProcessorSeq idleProcessors;						// list of idle processors associated with this cluster
	uBaseTaskSeq tasksOnCluster;						// list of tasks on this cluster
	uProcessorSeq processorsOnCluster;					// list of processors associated with this cluster
	unsigned int numProcessors;							// number of processors on cluster
	unsigned int stackSize;								// default stack size for tasks created on cluster

	uClusterDL wakeupList;								// double link field: list of clusters with wakeups

	// Make a pointer to allow static declaration for uniprocessor.
#if ! defined( __U_MULTI__ )
	static												// shared info on uniprocessor
#endif // ! __U_MULTI__
	UPP::uNBIO * NBIO;									// non-blocking I/O facilities

	// profiling : necessary for compatibility between non-profiling and profiling

	mutable uProfileClusterSampler * profileClusterSamplerInstance; // pointer to related profiling object

	static void wakeProcessor( uPid_t pid );
	void processorPause();
	void makeProcessorIdle( uProcessor & processor );
	void makeProcessorActive( uProcessor & processor );
	void makeProcessorActive();

	bool readyQueueEmpty() {
		return readyQueue->empty();
	} // uCluster::readyQueueEmpty

	void makeTaskReady( uBaseTask & readyTask );
	void makeTaskReady( uSequence<uBaseTaskDL> & readyQueue, unsigned int n );
	void readyQueueRemove( uBaseTaskDL * task );
	uBaseTask & readyQueueTryRemove();
	void taskAdd( uBaseTask & task );
	void taskRemove( uBaseTask & task );
	void taskReschedule( uBaseTask & task );
	virtual void processorAdd( uProcessor & processor );
	void processorRemove( uProcessor & processor );
#if defined( __U_MULTI__ )
	void processorPoke();
#endif // __U_MULTI__
	void createCluster( unsigned int stackSize, const char * name );

	int select( uIOClosure & closure, int rwe, timeval * timeout = nullptr ) {
		return NBIO->select( closure, rwe, timeout );
	} // uCluster::select
  public:
	uCluster( const uCluster & ) = delete;				// no copy
	uCluster( uCluster && ) = delete;
	uCluster & operator=( const uCluster & ) = delete;	// no assignment
	uCluster & operator=( uCluster && ) = delete;

	uCluster( unsigned int stackSize = uDefaultStackSize(), const char * name = "* unnamed*" );
	uCluster( const char * name );
	uCluster( uBaseSchedule<uBaseTaskDL> & ReadyQueue, unsigned int stackSize = uDefaultStackSize(), const char * name = "* unnamed*" );
	uCluster( uBaseSchedule<uBaseTaskDL> & ReadyQueue, const char * name = "* unnamed*" );
	virtual ~uCluster();

	const char * setName( const char * name ) {
		const char * prev = name;
		uCluster::name = name;
		return prev;
	} // uCluster::setName

	const char * getName() const {
		return
			uDEBUG(
				( name == nullptr || name == (const char *)-1 ) ? "* unknown*" : // storage might be scrubbed
			)
			name;
	} // uCluster::getName

	unsigned int setStackSize( unsigned int stackSize_ ) {
		unsigned int prev = stackSize;
		stackSize = stackSize_;
		return prev;
	} // uCluster::setStackSize

	unsigned int getStackSize() const {
		return stackSize;
	} // uCluster::getStackSize

	void taskResetPriority( uBaseTask & owner, uBaseTask & calling );
	void taskSetPriority( uBaseTask & owner, uBaseTask & calling );

	enum { ReadSelect = 1, WriteSelect = 2,  ExceptSelect = 4 };

	int select( int fd, int rwe, timeval * timeout = nullptr );

	int select( int nfds, fd_set * rfd, fd_set * wfd, fd_set * efd, timeval * timeout = nullptr ) {
		return NBIO->select( nfds, rfd, wfd, efd, timeout );
	} // uCluster::select

	const uBaseTaskSeq & getTasksOnCluster() {
		return tasksOnCluster;
	} // uCluster::getTasksOnCluster

	unsigned int getProcessors() const {
		return numProcessors;
	} // uCluster::getProcessors

	const uProcessorSeq & getProcessorsOnCluster() {
		return processorsOnCluster;
	} // uCluster::getProcessorsOnCluster

	void * operator new( size_t size ) {
		return ::memalign( 128, size );					// size of cache line to prevent false sharing
	} // uCluster::operator new
}; // uCluster


//######################### uBaseCoroutine (cont) #########################


inline uBaseCoroutine::uBaseCoroutine() : UPP::uMachContext( uThisCluster().getStackSize() ) {
	createCoroutine();
} // uBaseCoroutine::uBaseCoroutine


//######################### uPthreadable #########################


extern "C" {											// not all prototypes in pthread.h
	void _pthread_cleanup_push( _pthread_cleanup_buffer *, void (*) (void *), void * ) __THROW;
	void _pthread_cleanup_pop( _pthread_cleanup_buffer *, int ) __THROW;  
	int pthread_tryjoin_np( pthread_t, void **) __THROW;
	int pthread_getattr_np(pthread_t, pthread_attr_t*) __THROW;
	int pthread_attr_setstack( pthread_attr_t * attr, void * stackaddr, size_t stackSize ) __THROW;
	int pthread_attr_getstack( const pthread_attr_t * attr, void ** stackaddr, size_t * stackSize ) __THROW;
} // extern "C"


// uC++ tasks derived from the following have "almost" pthread capabilities

_Task uPthreadable {									// abstract class (inheritance only)
	friend void UPP::uMachContext::invokeTask( uBaseTask & ); // access: stop_unwinding
	friend int pthread_create( pthread_t * new_thread_id, const pthread_attr_t * attr, void * (* start_func)( void * ), void * arg ) __THROW;
	friend int pthread_attr_init( pthread_attr_t * attr ) __THROW;
	friend int pthread_attr_destroy( pthread_attr_t * attr ) __THROW;
	friend int pthread_attr_setscope( pthread_attr_t * attr, int contentionscope ) __THROW;
	friend int pthread_attr_getscope( const pthread_attr_t * attr, int * contentionscope ) __THROW;
	friend int pthread_attr_setdetachstate( pthread_attr_t * attr, int detachstate ) __THROW;
	friend int pthread_attr_getdetachstate( const pthread_attr_t * attr, int * detachstate ) __THROW;
	friend int pthread_attr_setstacksize( pthread_attr_t * attr, size_t stacksize ) __THROW;
	friend int pthread_attr_getstacksize( const pthread_attr_t * attr, size_t * stacksize ) __THROW;
	friend int pthread_attr_setstackaddr( pthread_attr_t * attr, void * stackaddr ) __THROW;
	friend int pthread_attr_getstackaddr( const pthread_attr_t * attr, void ** stackaddr ) __THROW;
	friend int pthread_attr_setstack( pthread_attr_t * attr, void * stackaddr, size_t stacksize ) __THROW;
	friend int pthread_attr_getstack( const pthread_attr_t * attr, void ** stackaddr, size_t * stacksize ) __THROW;
	friend int pthread_getattr_np( pthread_t threadID, pthread_attr_t * attr ) __THROW;
	friend int pthread_attr_setschedpolicy( pthread_attr_t * attr, int policy ) __THROW;
	friend int pthread_attr_getschedpolicy( const pthread_attr_t * attr, int * policy ) __THROW;
	friend int pthread_attr_setinheritsched( pthread_attr_t * attr, int inheritsched ) __THROW;
	friend int pthread_attr_getinheritsched( const pthread_attr_t * attr, int * inheritsched ) __THROW;
	friend int pthread_attr_setschedparam( pthread_attr_t * attr, const struct sched_param * param ) __THROW;
	friend int pthread_attr_getschedparam( const pthread_attr_t * attr, struct sched_param * param ) __THROW;
	friend void pthread_exit( void * status );			// access: joinval
	friend int pthread_join( pthread_t, void **);		// access: attr
	friend int pthread_tryjoin_np( pthread_t, void **) __THROW;
	friend int pthread_detach( pthread_t ) __THROW;
	friend void _pthread_cleanup_push( _pthread_cleanup_buffer *, void (*) (void *), void *) __THROW;
	friend void _pthread_cleanup_pop ( _pthread_cleanup_buffer *, int) __THROW;
	struct Pthread_attr_t {								// thread attributes
		int contentionscope;
		int detachstate;
		size_t stacksize;
		void * stackaddr;
		int policy;
		int inheritsched;
		struct sched_param param;
	} attr;

	static const Pthread_attr_t u_pthread_attr_defaults;

	uBaseCoroutine::Cleanup cleanup_handlers;
	pthread_t pthreadId_;								// used to allow joining with uPthreadables
	_Unwind_Exception uexc;
	bool stop_unwinding;								// indicates that forced unwinding should stop

	// uPthreadable( const uPthreadable & ) = delete;	// no copy
	// uPthreadable( uPthreadable && ) = delete;
	// uPthreadable & operator=( const uPthreadable & ) = delete; // no assignment
	// uPthreadable & operator=( uPthreadable && ) = delete;
	void createPthreadable( const pthread_attr_t * attr_ = nullptr );

	static Pthread_attr_t *& get( const pthread_attr_t * attr ) {
		return *((Pthread_attr_t **)attr);
	} // uPthreadable::get
  protected:
	void * joinval;										// pthreads return value
	pthread_attr_t pthread_attr;						// pthread attributes

	uPthreadable( const pthread_attr_t * attr ) :
			uBaseTask( attr != nullptr ? get( attr )->stackaddr : u_pthread_attr_defaults.stackaddr,
					   attr != nullptr ? get( attr )->stacksize : u_pthread_attr_defaults.stacksize ) {
		createPthreadable( attr );
	} // uPthreadable::uPthreadable

	uPthreadable() {									// same constructors as for uBaseTask
		createPthreadable();
	} // uPthreadable::uPthreadable

	uPthreadable( unsigned int stackSize ) : uBaseTask( stackSize ) {
		createPthreadable();
	} // uPthreadable::uPthreadable

	uPthreadable( void * storage, unsigned int storageSize ) : uBaseTask( storage, storageSize ) {
		createPthreadable();
	} // uPthreadable::uPthreadable

	uPthreadable( uCluster & cluster ) : uBaseTask( cluster ) {
		createPthreadable();
	} // uPthreadable::uPthreadable

	uPthreadable( uCluster & cluster, unsigned int stackSize ) : uBaseTask( cluster, stackSize ) {
		createPthreadable();
	} // uPthreadable::uPthreadable

	uPthreadable( uCluster & cluster, void * storage, unsigned int storageSize ) : uBaseTask( cluster, storage, storageSize ) {
		createPthreadable();
	} // uPthreadable::uPthreadable

	virtual void main() = 0;							// remove BaseTask from error message
  public:
	~uPthreadable();
	_Nomutex pthread_t pthreadId() { return pthreadId_; } // returns a pthread id for a uC++ task

	// exception handling

	_Event Failure : public uKernelFailure {
	  protected:
		Failure();
	}; // uPthreadable::Failure

	_Event CreationFailure : public uPthreadable::Failure {
	}; // uPthreadable::CreationFailure
  private:
	// The following routines should only have to be called by the original task owner.

	_Mutex void * join() {								// may only be accepted after the task which inherits
		return joinval;									// this terminates and turns into a monitor
	} // uPthreadable::join

	static void restart_unwinding( _Unwind_Reason_Code urc, _Unwind_Exception * e );
	static _Unwind_Reason_Code unwinder_cleaner( int version, _Unwind_Action ,_Unwind_Exception_Class, _Unwind_Exception *, _Unwind_Context *, void * );
	void do_unwind();
	void cleanup_pop( int ex );
	void cleanup_push( void (* routine)(void *), void * args, void * stackaddress );		
	uBaseCoroutine::PthreadCleanup * cleanupStackTop();
}; // uPthreadable


//######################### uMain #########################


_Task uMain : public uPthreadable {
	friend _Task uPthread;								// access: cleanup_handlers

	int argc;
	char ** argv, ** env;

	// A reference to a variable that holds the return code that the uMain task
	// returns to the OS.

	int & uRetCode;

	// Main routine for the first user task, declared here, defined by user.

	void main();
  public:
	uMain( int argc, char * argv[], char * env[], int & retcode );
	~uMain();
}; // uMain


//######################### uHeapControl #########################


namespace UPP {
	class uHeapControl {
		friend class UPP::uKernelBoot;					// access: startup, finishup
		friend class ::uBaseTask;						// access: prepareTask
		friend class UPP::PthreadLock;					// access: startup

		static void finishup();
		static void prepareTask( uBaseTask * task );
		static void startTask();
		static void finishTask();
		static void startup();

		static bool traceHeap_;							// trace allocations and deallocations
	  public:
		static bool initialized();

		static bool traceHeap() {
			return traceHeap_;
		} // uHeapControl::traceHeap

		static bool traceHeapOn() {
			bool temp = uHeapControl::traceHeap_;
			uHeapControl::traceHeap_ = true;
			return temp;
		} // uHeapControl::traceHeapOn

		static bool traceHeapOff() {
			bool temp = uHeapControl::traceHeap_;
			uHeapControl::traceHeap_ = false;
			return temp;
		} // uHeapControl::traceHeapOff

	  private:
		static bool prtHeapTerm_;						// print heap on termination
	  public:
		static bool prtHeapTerm() {
			return prtHeapTerm_;
		} // prtHeapTerm

		static bool prtHeapTermOn() {
			bool temp = prtHeapTerm_;
			prtHeapTerm_ = true;
			return temp;
		} // prtHeapTermOn

		static bool prtHeapTermOff() {
			bool temp = prtHeapTerm_;
			prtHeapTerm_ = false;
			return temp;
		} // prtHeapTermOff

	  private:
		static bool prtFree_;							// print free lists
	  public:
		static bool prtFree() {
			return prtFree_;
		} // prtFree

		static bool prtFreeOn() {
			bool temp = prtFree_;
			prtFree_ = true;
			return temp;
		} // prtFreeOn

		static bool prtFreeOff() {
			bool temp = prtFree_;
			prtFree_ = false;
			return temp;
		} // prtFreeOff
	}; // uHeapControl
} // UPP


//######################### Kernel Boot #########################


//#define __U_DEBUG_H__
//#include <uDebug.h>

namespace UPP {
	class uKernelBoot {
		static int count;

		static void startup();
		static void finishup();
	  public:
		uKernelBoot() {
			count += 1;
			if ( count == 1 ) {
				startup();
			} // if
			uDEBUGPRT( uDebugPrt( "(uKernelBoot &)%p.uKernelBoot\n", this ); )
		} // uKernelBoot::uKernelBoot

		~uKernelBoot() {
			uDEBUGPRT( uDebugPrt( "(uKernelBoot &)%p.~uKernelBoot\n", this ); )
			if ( count == 1 ) {
				finishup();
			} // if
			count -= 1;
		} // uKernelBoot::~uKernelBoot
	}; // uKernelBoot


	class uInitProcessorsBoot {
		static int count;

		static void startup();
		static void finishup();
	  public:
		uInitProcessorsBoot() {
			count += 1;
			if ( count == 1 ) {
				startup();
			} // if
			uDEBUGPRT( uDebugPrt( "(uInitProcessorsBoot &)%p.uInitProcessorsBoot\n", this ); )
		} // uInitProcessorsBoot::uInitProcessorsBoot

		~uInitProcessorsBoot() {
			uDEBUGPRT( uDebugPrt( "(uInitProcessorsBoot &)%p.~uInitProcessorsBoot\n", this ); )
			if ( count == 1 ) {
				finishup();
			} // if
			count -= 1;
		} // uInitProcessorsBoot::~uInitProcessorsBoot
	}; // uInitProcessorsBoot
} // UPP


#include <uBaseSelector.h>								// select statement


// debugging

#ifdef __U_DEBUG__
#include <uLocalDebugger.h>
#endif // __U_DEBUG__


// Create an instance in each translation unit, but only the first instance to execute performs the system bootstrap. Do
// not include instances in the kernel modules.

#ifndef __U_KERNEL__
#include <ios>
static std::ios_base::Init __ioinit;					// ensure streams are initialized before startup
static UPP::uKernelBoot uBootKernel;

#if __U_LOCALDEBUGGER_H__
static uLocalDebuggerBoot uBootLocalDebugger;
#endif // __U_LOCALDEBUGGER_H__

#if defined( __U_PROFILE__ )
#ifndef __U_PROFILEABLE_ONLY__
#include <uProfilerBoot.h>
static uProfilerBoot uBootProfiler;

#endif // __U_PROFILEABLE_ONLY__
#endif // __U_PROFILE__

static UPP::uInitProcessorsBoot uBootProcessorsInit;

#endif // __U_KERNEL__


#if __GNUC__ >= 7										// valid GNU compiler diagnostic ?
#endif // __GNUC__ >= 7
#pragma GCC diagnostic pop


// Local Variables: //
// compile-command: "make install" //
// End: //
