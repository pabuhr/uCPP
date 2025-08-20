// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uHeapLmmm.cc -- Lean Mean Malloc Machine - a runtime configurable replacement
//                 for malloc.
// 
// Author           : Peter A. Buhr
// Created On       : Sat Nov 11 16:07:20 1988
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Aug 15 18:47:32 2025
// Update Count     : 2483
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

//#define __U_DEBUG_H__									// turn off debug prints
#include <uDebug.h>										// access: uDebugWrite

#include <cstring>										// strlen, memset, memcpy
#include <climits>										// ULONG_MAX
#include <cerrno>										// errno, ENOMEM, EINVAL
#include <cassert>										// assert
#include <cstdint>										// uintptr_t, uint64_t, uint32_t
#include <unistd.h>										// STDERR_FILENO, sbrk, sysconf, write
#include <sys/mman.h>									// mmap, munmap
#include <sys/sysinfo.h>								// get_nprocs

#define str(s) #s
#define xstr(s) str(s)
#define WARNING( s ) xstr( GCC diagnostic ignored str( -W ## s ) )
#define NOWARNING( statement, warning ) \
	_Pragma( "GCC diagnostic push" ) \
	_Pragma( WARNING( warning ) ) \
	statement ;	\
	_Pragma ( "GCC diagnostic pop" )

#define __FASTLOOKUP__									// use O(1) table lookup from allocation size to bucket size for small allocations
#define __OWNERSHIP__									// return freed memory to owner thread
//#define __REMOTESPIN__									// toggle spinlock / lockfree queue
#if ! defined( __OWNERSHIP__ ) && defined( __REMOTESPIN__ )
#warning "REMOTESPIN is ignored without OWNERSHIP; suggest commenting out REMOTESPIN"
#endif // ! __OWNERSHIP__ && __REMOTESPIN__

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define CACHE_ALIGN 64
#define CALIGN __attribute__(( aligned(CACHE_ALIGN) ))


//######################### Helpers #########################


enum { BufSize = 1024 };

// std::min and std::lower_bound do not always inline, so substitute hand-coded versions.

#define Min( x, y ) (x < y ? x : y)

static inline __attribute__((always_inline)) size_t Bsearchl( unsigned int key, const unsigned int vals[], size_t dimension ) {
	size_t l = 0, m, h = dimension;
	while ( l < h ) {
		m = (l + h) / 2;
		if ( vals[m] < key ) {
			l = m + 1;
		} else {
			h = m;
		} // if
	} // while
	return l;
} // Bsearchl

// uSpinLock is used for mutual exclusion but the acquire/release must not rollforward because most of the critical
// sections are not interruptible. Hence, members acquire_/release_ are used.


//####################### Heap Statistics ####################


#ifdef __U_STATISTICS__
enum { CntTriples = 13 };								// number of counter triples
struct HeapStatistics {
	enum { MALLOC, AALLOC, CALLOC, MEMALIGN, AMEMALIGN, CMEMALIGN, RESIZE, REALLOC, FREE };
	union {
		// Statistic counters are unsigned long long int => use 64-bit counters on both 32 and 64 bit architectures.
		// On 32-bit architectures, the 64-bit counters are simulated with multi-precise 32-bit computations.
		struct {										// minimum qualification
			unsigned long long int malloc_calls, malloc_0_calls;
			unsigned long long int malloc_storage_request, malloc_storage_alloc;
			unsigned long long int aalloc_calls, aalloc_0_calls;
			unsigned long long int aalloc_storage_request, aalloc_storage_alloc;
			unsigned long long int calloc_calls, calloc_0_calls;
			unsigned long long int calloc_storage_request, calloc_storage_alloc;
			unsigned long long int memalign_calls, memalign_0_calls;
			unsigned long long int memalign_storage_request, memalign_storage_alloc;
			unsigned long long int amemalign_calls, amemalign_0_calls;
			unsigned long long int amemalign_storage_request, amemalign_storage_alloc;
			unsigned long long int cmemalign_calls, cmemalign_0_calls;
			unsigned long long int cmemalign_storage_request, cmemalign_storage_alloc;
			unsigned long long int resize_calls, resize_0_calls;
			unsigned long long int resize_storage_request, resize_storage_alloc;
			unsigned long long int realloc_calls, realloc_0_calls;
			unsigned long long int realloc_storage_request, realloc_storage_alloc;
			unsigned long long int realloc_copy, realloc_smaller;
			unsigned long long int realloc_align, realloc_0_fill;
			unsigned long long int free_calls, free_null_0_calls;
			unsigned long long int free_storage_request, free_storage_alloc;
			unsigned long long int remote_pushes, remote_pulls;
			unsigned long long int remote_storage_request, remote_storage_alloc;
			unsigned long long int mmap_calls, mmap_0_calls; // no zero calls
			unsigned long long int mmap_storage_request, mmap_storage_alloc;
			unsigned long long int munmap_calls, munmap_0_calls; // no zero calls
			unsigned long long int munmap_storage_request, munmap_storage_alloc;
		};
		struct {										// overlay for iteration
			unsigned long long int calls, calls_0;
			unsigned long long int request, alloc;
		} counters[CntTriples];
	};
}; // HeapStatistics

static_assert( sizeof(HeapStatistics) == CntTriples * sizeof(HeapStatistics::counters[0]),
			   "Heap statistics counter-triplets does not match with array size" );

static void HeapStatisticsCtor( HeapStatistics & stats ) {
	memset( &stats, '\0', sizeof(stats) );				// very fast
	// for ( unsigned int i = 0; i < CntTriples; i += 1 ) {
	// 	stats.counters[i].calls = stats.counters[i].calls_0 = stats.counters[i].request = stats.counters[i].alloc = 0;
	// } // for
} // HeapStatisticsCtor

static HeapStatistics & operator+=( HeapStatistics & lhs, const HeapStatistics & rhs ) {
	for ( unsigned int i = 0; i < CntTriples; i += 1 ) {
		lhs.counters[i].calls += rhs.counters[i].calls;
		lhs.counters[i].calls_0 += rhs.counters[i].calls_0;
		lhs.counters[i].request += rhs.counters[i].request;
		lhs.counters[i].alloc += rhs.counters[i].alloc;
	} // for
	return lhs;
} // ::operator+=
#endif // __U_STATISTICS__


//####################### Heap Structure ####################


struct Heap {
	struct FreeHeader;									// forward declaration

	struct Storage {
		struct Header {									// header
			union Kind {
				struct RealHeader {						// 4-byte word => 8-byte header, 8-byte word => 16-byte header
					union {
						// 2nd low-order bit => zero filled, 3rd low-order bit => mmapped
						FreeHeader * home;				// allocated block points back to home locations (must overlay alignment)
						size_t blockSize;				// size for munmap (must overlay alignment)
						Storage * next;					// freed block points to next freed block of same size
					};
					size_t size;						// allocation size in bytes
				} real; // RealHeader

				struct FakeHeader {
					uintptr_t alignment;				// 1st low-order bit => fake header & alignment
					uintptr_t offset;
				} fake; // FakeHeader
			} kind; // Kind
		} header; // Header

		char pad[uAlign() - sizeof( Header )];
		char data[0];									// storage
	}; // Storage

	static_assert( uAlign() >= sizeof( Storage ), "minimum alignment < sizeof( Storage )" );

	struct CALIGN FreeHeader {
		#ifdef __OWNERSHIP__
		#ifdef __REMOTESPIN__
		uNoCtor<uSpinLock, false> remoteLock;
		#endif // __REMOTESPIN__
		Storage * remoteList;							// other thread remote list
		#endif // __OWNERSHIP__

		Storage * freeList;								// thread free list
		Heap * homeManager;								// heap owner (free storage to bucket, from bucket to heap)
		size_t blockSize;								// size of allocations on this list
		#if defined( __U_STATISTICS__ )
		size_t allocations, reuses;
		#endif // __U_STATISTICS__
	}; // FreeHeader

	// Recursive definitions: HeapManager needs size of bucket array and bucket area needs sizeof HeapManager storage.
	// Break recursion by hardcoding number of buckets and statically checking number is correct after bucket array defined.
	enum { NoBucketSizes = 60 };						// number of bucket sizes

	FreeHeader freeLists[NoBucketSizes];				// buckets for different allocation sizes
	void * bufStart;									// start of current buffer
	size_t bufRemaining;								// remaining free storage in buffer

	#if defined( __U_STATISTICS__ ) || defined( __U_DEBUG__ )
	Heap * nextHeapManager;								// intrusive link of existing heaps; traversed to collect statistics or check unfreed storage
	#endif // __U_STATISTICS__ || __U_DEBUG__
	Heap * nextFreeHeapManager;							// intrusive link of free heaps from terminated threads; reused by new threads

	uDEBUG(	ptrdiff_t allocUnfreed; );					// running total of allocations minus frees; can be negative

	#ifdef __U_STATISTICS__
	HeapStatistics stats;								// local statistic table for this heap
	#endif // __U_STATISTICS__
}; // Heap


// Size of array must harmonize with NoBucketSizes and individual bucket sizes must be multiple of 16.
// Smaller multiples of 16 and powers of 2 are common allocation sizes, so make them generate the minimum required bucket size.
static const unsigned int CALIGN bucketSizes[] = {		// different bucket sizes
	// There is no 0-sized bucket becasue it is better to create a 16 byte bucket for rare malloc(0), which can be
	// reused later by a 16-byte allocation.
	16 + sizeof(Heap::Storage), 32 + sizeof(Heap::Storage), 48 + sizeof(Heap::Storage), 64 + sizeof(Heap::Storage), // 4
	96 + sizeof(Heap::Storage), 128 + sizeof(Heap::Storage), // 2
	160, 192, 224, 256 + sizeof(Heap::Storage), // 4
	320, 384, 448, 512 + sizeof(Heap::Storage), // 4
	640, 768, 896, 1'024 + sizeof(Heap::Storage), // 4
	1'536, 2'048 + sizeof(Heap::Storage), // 2
	2'560, 3'072, 3'584, 4'096 + sizeof(Heap::Storage), // 4
	6'144, 8'192 + sizeof(Heap::Storage), // 2
	10'240, 12'288, 14'336, 16'384 + sizeof(Heap::Storage), // 4
	20'480, 24'576, 28'672, 32'768 + sizeof(Heap::Storage), // 4
	40'960, 49'152, 57'344, 65'536 + sizeof(Heap::Storage), // 4
	81'920, 98'304, 114'688, 131'072 + sizeof(Heap::Storage), // 4
	163'840, 196'608, 229'376, 262'144 + sizeof(Heap::Storage), // 4
	327'680, 393'216, 458'752, 524'288 + sizeof(Heap::Storage), // 4
	786'432, 1'048'576 + sizeof(Heap::Storage), // 2
	1'572'864, 2'097'152 + sizeof(Heap::Storage), // 2
	3'145'728, 4'194'304 + sizeof(Heap::Storage), // 2
	6'291'456, 8'388'608 + sizeof(Heap::Storage), 12'582'912, 16'777'216 + sizeof(Heap::Storage), // 4
};

static_assert( Heap::NoBucketSizes == sizeof(bucketSizes) / sizeof(bucketSizes[0] ), "size of bucket array is wrong" );

#ifdef __FASTLOOKUP__
enum { LookupSizes = 65'536 + sizeof(Heap::Storage) };	// number of fast lookup sizes
static unsigned char CALIGN lookup[LookupSizes];		// O(1) lookup for small sizes
#endif // __FASTLOOKUP__


// Manipulate sticky bits stored in unused 3 low-order bits of an address.
//   bit0 => alignment => fake header
//   bit1 => zero filled (calloc)
//   bit2 => mapped allocation versus sbrk
#define StickyBits( header ) (((header)->kind.real.blockSize & 0x7))
#define ClearStickyBits( addr ) (decltype(addr))((uintptr_t)(addr) & ~7)
#define MarkAlignmentBit( alignment ) ((alignment) | 1)
#define AlignmentBit( header ) ((((header)->kind.fake.alignment) & 1))
#define ClearAlignmentBit( header ) (((header)->kind.fake.alignment) & ~1)
#define ZeroFillBit( header ) ((((header)->kind.real.blockSize) & 2))
#define ClearZeroFillBit( header ) ((((header)->kind.real.blockSize) &= ~2))
#define MarkZeroFilledBit( header ) ((header)->kind.real.blockSize |= 2)
#define MmappedBit( header ) ((((header)->kind.real.blockSize) & 4))
#define MarkMmappedBit( size ) ((size) | 4)


enum {
	// The default extension heap amount in units of bytes. When the current heap reaches the brk address, the brk
	// address is extended by the extension amount.
	__DEFAULT_HEAP_EXTEND__ = 8 * 1024 * 1024,

	// The mmap crossover point during allocation. Allocations less than this amount are allocated from buckets; values
	// greater than or equal to this value are mmap from the operating system.
	__DEFAULT_MMAP_START__ = 8 * 1024 * 1024 + sizeof(Heap::Storage),

	// The default unfreed storage amount in units of bytes. When the program ends it subtracts this amount from
	// the malloc/free counter to adjust for storage the program does not free.
	__DEFAULT_HEAP_UNFREED__ = 0
}; // enum


struct HeapMaster {
	uNoCtor<uSpinLock, false> extLock;					// protects allocation-buffer extension
	uNoCtor<uSpinLock, false> mgrLock;					// protects freeHeapManagersList, heapManagersList, heapManagersStorage, heapManagersStorageEnd

	void * sbrkStart;									// start of sbrk storage
	void * sbrkEnd;										// end of sbrk area (logical end of heap)
	size_t sbrkRemaining;								// amount of free storage at end of sbrk area
	size_t sbrkExtend;									// sbrk extend amount
	size_t pageSize;									// architecture pagesize
	size_t mmapStart;									// cross over point for mmap
	size_t maxBucketsUsed;								// maximum number of buckets in use

	#if defined( __U_STATISTICS__ ) || defined( __U_DEBUG__ )
	Heap * heapManagersList;							// heap-stack head
	#endif // __U_STATISTICS__ || __U_DEBUG__
	Heap * freeHeapManagersList;						// free-stack head

	// Heap superblocks are not linked; heaps in superblocks are linked via intrusive links.
	Heap * heapManagersStorage;							// next heap to use in heap superblock
	Heap * heapManagersStorageEnd;						// logical heap outside of superblock's end

	uDEBUG(	ptrdiff_t allocUnfreed; );					// running total of allocations minus frees; can be negative

	#ifdef __U_STATISTICS__
	HeapStatistics stats;								// global stats for thread-local heaps to add there counters when exiting
	unsigned long long int nremainder, remainder;		// counts mostly unusable storage at the end of a thread's reserve block
	unsigned long long int threads_started, threads_exited; // counts threads that have started and exited
	unsigned long long int reused_heap, new_heap;		// counts reusability of heaps
	unsigned long long int sbrk_calls;
	unsigned long long int sbrk_storage;
	int stats_fd;
	#endif // __U_STATISTICS__
}; // HeapMaster


static volatile bool heapMasterBootFlag = false;		// trigger for first heap
static HeapMaster heapMaster;							// program global


// Thread-local storage is allocated lazily when the storage is accessed. No tls_model("initial-exec") because uC++ is
// statically linked.
static __U_THREAD_LOCAL__ size_t PAD1 CALIGN __attribute__(( unused )); // protect prior false sharing
static __U_THREAD_LOCAL__ Heap * heapManager CALIGN = (Heap *)1; // singleton
static __U_THREAD_LOCAL__ size_t PAD2 CALIGN __attribute__(( unused )); // protect further false sharing


static void heapMasterCtor( void ) {
	// Singleton pattern to initialize heap master

	heapMaster.pageSize = sysconf( _SC_PAGESIZE );

	heapMaster.extLock.ctor();
	heapMaster.mgrLock.ctor();

	char * end = (char *)sbrk( 0 );
	heapMaster.sbrkStart = heapMaster.sbrkEnd = sbrk( (char *)uCeiling( (long unsigned int)end, uAlign() ) - end ); // move start of heap to multiple of alignment
	heapMaster.sbrkRemaining = 0;
	heapMaster.sbrkExtend = malloc_extend();
	heapMaster.mmapStart = malloc_mmap_start();

	// find the closest bucket size less than or equal to the mmapStart size
	heapMaster.maxBucketsUsed = Bsearchl( heapMaster.mmapStart, bucketSizes, Heap::NoBucketSizes ); // binary search

	assert( (heapMaster.mmapStart >= heapMaster.pageSize) && (bucketSizes[Heap::NoBucketSizes - 1] >= heapMaster.mmapStart) );
	assert( heapMaster.maxBucketsUsed < Heap::NoBucketSizes ); // subscript failure ?
	assert( heapMaster.mmapStart <= bucketSizes[heapMaster.maxBucketsUsed] ); // search failure ?

	#if defined( __U_STATISTICS__ ) || defined( __U_DEBUG__ )
	heapMaster.heapManagersList = nullptr;
	#endif // __U_STATISTICS__ || __U_DEBUG__
	heapMaster.freeHeapManagersList = nullptr;

	heapMaster.heapManagersStorage = nullptr;
	heapMaster.heapManagersStorageEnd = nullptr;

	#ifdef __U_STATISTICS__
	HeapStatisticsCtor( heapMaster.stats );				// clear statistic counters
	heapMaster.nremainder = heapMaster.remainder = 0;
	heapMaster.threads_started = heapMaster.threads_exited = 0;
	heapMaster.reused_heap = heapMaster.new_heap = 0;
	heapMaster.sbrk_calls = heapMaster.sbrk_storage = 0;
	heapMaster.stats_fd = STDERR_FILENO;
	#endif // __U_STATISTICS__

	uDEBUG(	heapMaster.allocUnfreed = 0; );

	#ifdef __FASTLOOKUP__
	for ( unsigned int i = 0, idx = 0; i < LookupSizes; i += 1 ) {
		if ( i > bucketSizes[idx] ) idx += 1;
		lookup[i] = idx;
		assert( i <= bucketSizes[idx] ||				// overflow buckets ?
				i > bucketSizes[idx - 1] );				// overlapping bucket sizes ?
	} // for
	#endif // __FASTLOOKUP__

	heapMasterBootFlag = true;
} // HeapMaster::heapMasterCtor


#define NO_MEMORY_MSG "insufficient heap memory available to allocate %zd new bytes."

static Heap * getHeap( void ) {
	Heap * heap;
	if ( heapMaster.freeHeapManagersList ) {			// free heap for reused ?
		// pop heap from stack of free heaps for reusability
		heap = heapMaster.freeHeapManagersList;
		heapMaster.freeHeapManagersList = heap->nextFreeHeapManager;

		#ifdef __U_STATISTICS__
		heapMaster.reused_heap += 1;
		#endif // __U_STATISTICS__
	} else {											// free heap not found, create new
		// Heap size is about 12K, FreeHeader (128 bytes because of cache alignment) * NoBucketSizes (91) => 128 heaps *
		// 12K ~= 120K byte superblock.  Where 128-heap superblock handles a medium sized multi-processor server.
		size_t remaining = heapMaster.heapManagersStorageEnd - heapMaster.heapManagersStorage; // remaining free heaps in superblock
		if ( ! heapMaster.heapManagersStorage || remaining == 0 ) {
			// Each block of heaps is a multiple of the number of cores on the computer.
			int dimension = get_nprocs();				// get_nprocs_conf does not work
			size_t size = dimension * sizeof( Heap );

			heapMaster.heapManagersStorage = (Heap *)mmap( 0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
			if ( UNLIKELY( heapMaster.heapManagersStorage == MAP_FAILED ) ) { // failed ?
				if ( errno == ENOMEM ) abort( NO_MEMORY_MSG, size ); // no memory
				// Do not call strerror( errno ) as it may call malloc.
				abort( "attempt to allocate block of heaps of size %zu bytes and mmap failed with errno %d.", size, errno );
			} // if
			heapMaster.heapManagersStorageEnd = &heapMaster.heapManagersStorage[dimension]; // outside array
		} // if

		heap = heapMaster.heapManagersStorage;
		heapMaster.heapManagersStorage = heapMaster.heapManagersStorage + 1; // bump next heap

		#if defined( __U_STATISTICS__ ) || defined( __U_DEBUG__ )
		heap->nextHeapManager = heapMaster.heapManagersList;
		heapMaster.heapManagersList = heap;
		#endif // __U_STATISTICS__ || __U_DEBUG__

		#ifdef __U_STATISTICS__
		heapMaster.new_heap += 1;
		#endif // __U_STATISTICS__

		for ( unsigned int j = 0; j < Heap::NoBucketSizes; j += 1 ) { // initialize free lists
			#ifdef __OWNERSHIP__
			#ifdef __REMOTESPIN__
			heap->freeLists[j].remoteLock.ctor();
			#endif // __REMOTESPIN__
			heap->freeLists[j].remoteList = nullptr;
			#endif // __OWNERSHIP__

			heap->freeLists[j].freeList = nullptr;
			heap->freeLists[j].homeManager = heap;
			heap->freeLists[j].blockSize = bucketSizes[j];

			#if defined( __U_STATISTICS__ )
			heap->freeLists[j].allocations = 0;
			heap->freeLists[j].reuses = 0;
			#endif // __U_STATISTICS__
		} // for

		heap->bufStart = nullptr;
		heap->bufRemaining = 0;
		heap->nextFreeHeapManager = nullptr;

		uDEBUG( heap->allocUnfreed = 0; );
	} // if

	return heap;
} // HeapMaster::getHeap


// Always called from uKernelModule::startThread.
__attribute__(( visibility ("hidden") ))
void heapManagerDtor( void ) {							// called by uKernelModule::startThread
	assert( heapManager );

	heapMaster.mgrLock->acquire_( true );				// protect heapMaster counters

	// push heap onto stack of free heaps for reusability
	heapManager->nextFreeHeapManager = heapMaster.freeHeapManagersList;
	heapMaster.freeHeapManagersList = heapManager;

	#ifdef __U_STATISTICS__
	heapMaster.stats += heapManager->stats;				// retain this heap's statistics
	HeapStatisticsCtor( heapManager->stats );			// reset heap counters for next usage
	heapMaster.threads_exited += 1;
	#endif // __U_STATISTICS__

	heapMaster.mgrLock->release_( true );
} // heapManagerDtor


// Always called from uKernelModule::startThread.
__attribute__(( visibility ("hidden") ))				// called by uKernelModule::startThread
void heapManagerCtor( void ) {
	if ( UNLIKELY( ! heapMasterBootFlag ) ) heapMasterCtor();
	assert( heapManager );

	heapMaster.mgrLock->acquire_( true );				// protect heapMaster counters

	// get storage for heap manager

	heapManager = getHeap();

	#ifdef __U_STATISTICS__
	HeapStatisticsCtor( heapManager->stats );			// heap local
	heapMaster.threads_started += 1;
	#endif // __U_STATISTICS__

	heapMaster.mgrLock->release_( true );
} // heapManagerCtor


//####################### Memory Allocation Routines Helpers ####################


void UPP::uHeapControl::startup( void ) {				// singleton => called once at start of program
	if ( ! heapMasterBootFlag ) { heapManagerCtor(); }	// sanity check
	uDEBUG( heapManager->allocUnfreed = 0; );			// clear prior allocation counts
} // uHeapControl::startup


void UPP::uHeapControl::shutdown( void ) {				// singleton => called once at end of program
	if ( getenv( "MALLOC_STATS" ) ) {					// check for external printing
		malloc_stats();

		#ifdef __U_STATISTICS__
		char helpText[128];
		int len = snprintf( helpText, sizeof(helpText), "\nFree Bucket Usage: (bucket-size/allocations/reuses)\n" );
		int unused __attribute__(( unused )) = write( STDERR_FILENO, helpText, len ); // file might be closed

		size_t th = 0, total = 0;
		// Heap list is a stack, so last heap is program main.
		for ( Heap * heap = heapMaster.heapManagersList; heap; heap = heap->nextHeapManager, th += 1 ) {
			enum { Columns = 8 };
			len = snprintf( helpText, sizeof(helpText), "Heap %'zd\n", th );
			unused = write( STDERR_FILENO, helpText, len ); // file might be closed
			for ( size_t b = 0, c = 0; b < Heap::NoBucketSizes; b += 1 ) {
				if ( heap->freeLists[b].allocations != 0 ) {
					total += heap->freeLists[b].blockSize * heap->freeLists[b].allocations;
					len = snprintf( helpText, sizeof(helpText), "%'zd/%'zd/%'zd, ",
									heap->freeLists[b].blockSize, heap->freeLists[b].allocations, heap->freeLists[b].reuses );
					unused = write( STDERR_FILENO, helpText, len ); // file might be closed
					if ( ++c % Columns == 0 )
						unused = write( STDERR_FILENO, "\n", 1 ); // file might be closed
				} // if
			} // for
			unused = write( STDERR_FILENO, "\n", 1 );	// file might be closed
			#ifdef __U_DEBUG__
			len = snprintf( helpText, sizeof(helpText), "allocUnfreed storage %'zd\n", heap->allocUnfreed );
			unused = write( STDERR_FILENO, helpText, len ); // file might be closed
			#endif // __U_DEBUG__
		} // for
		len = snprintf( helpText, sizeof(helpText), "Total bucket storage %'zd\n", total );
		unused = write( STDERR_FILENO, helpText, len ); // file might be closed
		#endif // __U_STATISTICS__
	} // if

	#ifdef __U_DEBUG__
	// allocUnfreed is set to 0 when a heap is created and it accumulates any unfreed storage during its multiple thread
	// usages.  At the end, add up each heap allocUnfreed value across all heaps to get the total unfreed storage.
	ptrdiff_t allocUnfreed = heapMaster.allocUnfreed;
	uDEBUGPRT( uDebugPrt( "shutdown1 %jd\n", heapMaster.allocUnfreed ) );
	for ( Heap * heap = heapMaster.heapManagersList; heap; heap = heap->nextHeapManager ) {
		uDEBUGPRT( uDebugPrt( "shutdown2 %p %jd\n", heap, heap->allocUnfreed ) );
		allocUnfreed += heap->allocUnfreed;
	} // for

	allocUnfreed -= malloc_unfreed();					// subtract any user specified unfreed storage
	uDEBUGPRT( uDebugPrt( "shutdown3 %td %zd\n", allocUnfreed, malloc_unfreed() ) );
	if ( allocUnfreed > 0 ) {
		// DO NOT USE STREAMS AS THEY MAY BE UNAVAILABLE AT THIS POINT.
		char helpText[BufSize];
		int len = snprintf( helpText, sizeof(helpText), "**** Warning **** (UNIX pid:%ld) : program terminating with %td(%#tx) bytes of storage allocated but not freed.\n"
							"Possible cause is unfreed storage allocated by the program or system/library routines called from the program.\n",
							(long int)getpid(), allocUnfreed, allocUnfreed ); // always print the UNIX pid
		uDebugWrite( STDERR_FILENO, helpText, len );	// file might be closed
	} // if
	#endif // __U_DEBUG__
} // uHeapControl::shutdown


void UPP::uHeapControl::prepareTask( uBaseTask * /* task */ ) {
} // uHeapControl::prepareTask


#ifdef __U_STATISTICS__
#define prtFmt \
	"\nPID: %d Heap%s statistics: (storage request/allocation)\n" \
	"  malloc    >0 calls %'llu; 0 calls %'llu; storage %'llu/%'llu bytes\n" \
	"  aalloc    >0 calls %'llu; 0 calls %'llu; storage %'llu/%'llu bytes\n" \
	"  calloc    >0 calls %'llu; 0 calls %'llu; storage %'llu/%'llu bytes\n" \
	"  memalign  >0 calls %'llu; 0 calls %'llu; storage %'llu/%'llu bytes\n" \
	"  amemalign >0 calls %'llu; 0 calls %'llu; storage %'llu/%'llu bytes\n" \
	"  cmemalign >0 calls %'llu; 0 calls %'llu; storage %'llu/%'llu bytes\n" \
	"  resize    >0 calls %'llu; 0 calls %'llu; storage %'llu/%'llu bytes\n" \
	"  realloc   >0 calls %'llu; 0 calls %'llu; storage %'llu/%'llu bytes\n" \
	"            copies %'llu; smaller %'llu; alignment %'llu; 0 fill %'llu\n" \
	"  free      !null calls %'llu; null/0 calls %'llu; storage %'llu/%'llu bytes\n" \
	"  remote    pushes %'llu; pulls %'llu; storage %'llu/%'llu bytes\n" \
	"  sbrk      calls %'llu; storage %'llu bytes\n" \
	"  mmap      calls %'llu; storage %'llu/%'llu bytes\n" \
	"  munmap    calls %'llu; storage %'llu/%'llu bytes\n" \
	"  remainder calls %'llu; storage %'llu bytes\n" \
	"  threads   started %'llu; exited %'llu\n" \
	"  heaps     new %'llu; reused %'llu\n"

// Use "write" because streams may be shutdown when calls are made.
static int printStats( HeapStatistics & stats, const char * title = "" ) { // see malloc_stats
	char helpText[sizeof(prtFmt) + 1024];				// space for message and values
	return uDebugPrtBuf2( heapMaster.stats_fd, helpText, sizeof(helpText), prtFmt,	getpid(), title,
		stats.malloc_calls, stats.malloc_0_calls, stats.malloc_storage_request, stats.malloc_storage_alloc,
		stats.aalloc_calls, stats.aalloc_0_calls, stats.aalloc_storage_request, stats.aalloc_storage_alloc,
		stats.calloc_calls, stats.calloc_0_calls, stats.calloc_storage_request, stats.calloc_storage_alloc,
		stats.memalign_calls, stats.memalign_0_calls, stats.memalign_storage_request, stats.memalign_storage_alloc,
		stats.amemalign_calls, stats.amemalign_0_calls, stats.amemalign_storage_request, stats.amemalign_storage_alloc,
		stats.cmemalign_calls, stats.cmemalign_0_calls, stats.cmemalign_storage_request, stats.cmemalign_storage_alloc,
		stats.resize_calls, stats.resize_0_calls, stats.resize_storage_request, stats.resize_storage_alloc,
		stats.realloc_calls, stats.realloc_0_calls, stats.realloc_storage_request, stats.realloc_storage_alloc,
		stats.realloc_copy, stats.realloc_smaller, stats.realloc_align, stats.realloc_0_fill,
		stats.free_calls, stats.free_null_0_calls, stats.free_storage_request, stats.free_storage_alloc,
		stats.remote_pushes, stats.remote_pulls, stats.remote_storage_request, stats.remote_storage_alloc,
		heapMaster.sbrk_calls, heapMaster.sbrk_storage,
		stats.mmap_calls, stats.mmap_storage_request, stats.mmap_storage_alloc,
		stats.munmap_calls, stats.munmap_storage_request, stats.munmap_storage_alloc,
		heapMaster.nremainder, heapMaster.remainder,
		heapMaster.threads_started, heapMaster.threads_exited,
		heapMaster.new_heap, heapMaster.reused_heap
	);
} // printStats

#define prtFmtXML \
	"<malloc version=\"1\">\n" \
	"<heap nr=\"0\">\n" \
	"<sizes>\n" \
	"</sizes>\n" \
	"<total type=\"malloc\" >0 count=\"%'llu;\" 0 count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"aalloc\" >0 count=\"%'llu;\" 0 count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"calloc\" >0 count=\"%'llu;\" 0 count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"memalign\" >0 count=\"%'llu;\" 0 count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"amemalign\" >0 count=\"%'llu;\" 0 count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"cmemalign\" >0 count=\"%'llu;\" 0 count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"resize\" >0 count=\"%'llu;\" 0 count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"realloc\" >0 count=\"%'llu;\" 0 count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"       \" copy count=\"%'llu;\" smaller count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"free\" !null=\"%'llu;\" 0 null/0=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"remote\" pushes=\"%'llu;\" 0 pulls=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"sbrk\" count=\"%'llu;\" size=\"%'llu\"/> bytes\n" \
	"<total type=\"mmap\" count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"munmap\" count=\"%'llu;\" size=\"%'llu/%'llu\"/> bytes\n" \
	"<total type=\"remainder\" count=\"%'llu;\" size=\"%'llu\"/> bytes\n" \
	"<total type=\"threads\" started=\"%'llu;\" exited=\"%'llu\"/>\n" \
	"<total type=\"heaps\" new=\"%'llu;\" reused=\"%'llu\"/>\n" \
	"</malloc>"

static int printStatsXML( HeapStatistics & stats, FILE * stream ) { // see malloc_info
	char helpText[sizeof(prtFmtXML) + 1024];			// space for message and values
	return uDebugPrtBuf2( fileno( stream ), helpText, sizeof(helpText), prtFmtXML,
		stats.malloc_calls, stats.malloc_0_calls, stats.malloc_storage_request, stats.malloc_storage_alloc,
		stats.aalloc_calls, stats.aalloc_0_calls, stats.aalloc_storage_request, stats.aalloc_storage_alloc,
		stats.calloc_calls, stats.calloc_0_calls, stats.calloc_storage_request, stats.calloc_storage_alloc,
		stats.memalign_calls, stats.memalign_0_calls, stats.memalign_storage_request, stats.memalign_storage_alloc,
		stats.amemalign_calls, stats.amemalign_0_calls, stats.amemalign_storage_request, stats.amemalign_storage_alloc,
		stats.cmemalign_calls, stats.cmemalign_0_calls, stats.cmemalign_storage_request, stats.cmemalign_storage_alloc,
		stats.resize_calls, stats.resize_0_calls, stats.resize_storage_request, stats.resize_storage_alloc,
		stats.realloc_calls, stats.realloc_0_calls, stats.realloc_storage_request, stats.realloc_storage_alloc,
		stats.realloc_copy, stats.realloc_smaller, stats.realloc_align, stats.realloc_0_fill,
		stats.free_calls, stats.free_null_0_calls, stats.free_storage_request, stats.free_storage_alloc,
		stats.remote_pushes, stats.remote_pulls, stats.remote_storage_request, stats.remote_storage_alloc,
		heapMaster.sbrk_calls, heapMaster.sbrk_storage,
		stats.mmap_calls, stats.mmap_storage_request, stats.mmap_storage_alloc,
		stats.munmap_calls, stats.munmap_storage_request, stats.munmap_storage_alloc,
		heapMaster.nremainder, heapMaster.remainder,
		heapMaster.threads_started, heapMaster.threads_exited,
		heapMaster.new_heap, heapMaster.reused_heap
	);
} // printStatsXML

static HeapStatistics & collectStats( HeapStatistics & stats ) {
	heapMaster.mgrLock->acquire();

	// Accumulate the heap master and all active thread heaps.
	stats += heapMaster.stats;
	for ( Heap * heap = heapMaster.heapManagersList; heap; heap = heap->nextHeapManager ) {
		stats += heap->stats;							// calls HeapStatistics +=
	} // for

	heapMaster.mgrLock->release();
	return stats;
} // collectStats

static void clearStats( void ) {
	heapMaster.mgrLock->acquire();

	// Zero the heap master and all active thread heaps.
	HeapStatisticsCtor( heapMaster.stats );
	for ( Heap * heap = heapMaster.heapManagersList; heap; heap = heap->nextHeapManager ) {
		HeapStatisticsCtor( heap->stats );
	} // for

	heapMaster.mgrLock->release();
} // clearStats
#endif // __U_STATISTICS__

static inline __attribute__((always_inline)) bool setMmapStart( size_t value ) { // true => mmapped, false => sbrk
  if ( value < heapMaster.pageSize || bucketSizes[Heap::NoBucketSizes - 1] < value ) return false;
	heapMaster.mmapStart = value;						// set global

	// find the closest bucket size less than or equal to the mmapStart size
	heapMaster.maxBucketsUsed = Bsearchl( heapMaster.mmapStart, bucketSizes, Heap::NoBucketSizes ); // binary search

	assert( heapMaster.maxBucketsUsed < Heap::NoBucketSizes ); // subscript failure ?
	assert( heapMaster.mmapStart <= bucketSizes[heapMaster.maxBucketsUsed] ); // search failure ?
	return true;
} // setMmapStart


// <-------+----------------------------------------------------> bsize (bucket size)
// |header |addr
//==================================================================================
//               alignment/offset |
// <-----------------<------------+-----------------------------> bsize (bucket size)
//                   |fake-header | addr
#define HeaderAddr( addr ) ((Heap::Storage::Header *)( (char *)addr - sizeof(Heap::Storage) ))
#define RealHeader( header ) ((Heap::Storage::Header *)((char *)header - header->kind.fake.offset))

// <-------<<--------------------- dsize ---------------------->> bsize (bucket size)
// |header |addr
//==================================================================================
//               alignment/offset |
// <------------------------------<<---------- dsize --------->>> bsize (bucket size)
//                   |fake-header |addr
#define DataStorage( bsize, addr, header ) (bsize - ( (char *)addr - (char *)header ))


static inline __attribute__((always_inline)) void checkAlign( size_t alignment ) {
	if ( UNLIKELY( alignment < uAlign() || ! uPow2( alignment ) ) ) {
		abort( "alignment %zu for memory allocation is less than %d and/or not a power of 2.", alignment, uAlign() );
	} // if
} // checkAlign


static inline __attribute__((always_inline)) void checkHeader( bool check, const char name[], void * addr ) {
	if ( UNLIKELY( check ) ) {							// bad address ?
		abort( "attempt to %s storage %p with address outside the heap range %p<->%p.\n"
			   "Possible cause is duplicate free on same block or overwriting of memory.",
			   name, addr, heapMaster.sbrkStart, heapMaster.sbrkEnd );
	} // if
} // checkHeader


static inline __attribute__((always_inline)) void fakeHeader( Heap::Storage::Header *& header, size_t & alignment ) {
	if ( UNLIKELY( AlignmentBit( header ) ) ) {			// fake header ?
		alignment = ClearAlignmentBit( header );		// clear flag from value
		uDEBUG( checkAlign( alignment ); );				// check alignment
		header = RealHeader( header );					// backup from fake to real header
	} else {
		alignment = uAlign();							// => no fake header
	} // if
} // fakeHeader


static inline __attribute__((always_inline)) bool headers( const char name[] __attribute__(( unused )), void * addr, Heap::Storage::Header *& header,
					 Heap::FreeHeader *& freeHead, size_t & size, size_t & alignment ) {
	header = HeaderAddr( addr );

	uDEBUG( if ( LIKELY( ! MmappedBit( header ) ) ) checkHeader( header < heapMaster.sbrkStart, name, addr ); ); // bad low address ?

	if ( LIKELY( ! StickyBits( header ) ) ) {			// no sticky bits ?
		freeHead = header->kind.real.home;
		alignment = uAlign();
	} else {
		fakeHeader( header, alignment );
		if ( UNLIKELY( MmappedBit( header ) ) ) {		// mapped storage ?
			assert( addr < heapMaster.sbrkStart || heapMaster.sbrkEnd < addr );
			size = ClearStickyBits( header->kind.real.blockSize ); // mmap size
			freeHead = nullptr;							// prevent uninitialized warning
			return true;
		} // if

		freeHead = ClearStickyBits( header->kind.real.home );
	} // if
	size = freeHead->blockSize;

	#ifdef __U_DEBUG__
	checkHeader( header < heapMaster.sbrkStart || heapMaster.sbrkEnd < header, name, addr ); // bad address ? (offset could be + or -)

	Heap * homeManager;
	if ( UNLIKELY( freeHead == nullptr || // freed and only free-list node => null link
				   // freed and link points at another free block not to a bucket in the bucket array.
				   (homeManager = freeHead->homeManager, freeHead < &homeManager->freeLists[0] ||
					&homeManager->freeLists[Heap::NoBucketSizes] <= freeHead ) ) ) {
		abort( "attempt to %s storage %p with corrupted header.\n"
			   "Possible cause is duplicate free on same block or overwriting of header information.",
			   name, addr );
	} // if
	#endif // __U_DEBUG__

	return false;
} // headers


static inline __attribute__((always_inline)) void * master_extend( size_t size ) {
	heapMaster.extLock->acquire_( true );

	ptrdiff_t rem = heapMaster.sbrkRemaining - size;
	if ( UNLIKELY( rem < 0 ) ) {						// negative ?
		// If the size requested is bigger than the current remaining storage, increase the size of the heap.
		size_t increase = uCeiling( size > heapMaster.sbrkExtend ? size : heapMaster.sbrkExtend, uAlign() );
		if ( UNLIKELY( sbrk( increase ) == (void *)-1 ) ) { heapMaster.extLock->release_( true ); return nullptr; } // failed, no memory, errnor == ENOMEM
		rem = heapMaster.sbrkRemaining + increase - size;

		#ifdef __U_STATISTICS__
		heapMaster.sbrk_calls += 1;
		heapMaster.sbrk_storage += increase;
		#endif // __U_STATISTICS__
	} // if

	void * newblock = (Heap::Storage *)heapMaster.sbrkEnd;
	heapMaster.sbrkRemaining = rem;
	heapMaster.sbrkEnd = (char *)heapMaster.sbrkEnd + size;

	heapMaster.extLock->release_( true );
	return newblock;
} // master_extend


__attribute__(( noinline ))
static void * manager_extend( size_t size ) {
	// If the size requested is bigger than the current remaining reserve, so increase the reserve.
	size_t increase = uCeiling( size > ( heapMaster.sbrkExtend / 16 ) ? size : ( heapMaster.sbrkExtend / 16 ), uAlign() );
	void * newblock = master_extend( increase );
	if ( UNLIKELY( newblock == nullptr ) ) return nullptr;

	// Check if the new reserve block is contiguous with the old block (The only good storage is contiguous storage!)
	// For sequential programs, this check is always true.
	if ( newblock != (char *)heapManager->bufStart + heapManager->bufRemaining ) {
		// Otherwise, find the closest bucket size to the remaining storage in the reserve block and chain it onto
		// that free list. Distributing the storage across multiple free lists is an option but takes time.
		ptrdiff_t rem = heapManager->bufRemaining;		// positive

		if ( (decltype(bucketSizes[0]))rem >= bucketSizes[0] ) { // minimal size ? otherwise ignore
			#ifdef __U_STATISTICS__
			heapMaster.nremainder += 1;
			heapMaster.remainder += rem;
			#endif // __U_STATISTICS__

			Heap::FreeHeader * freeHead =
			#ifdef __FASTLOOKUP__
				rem < LookupSizes ? &(heapManager->freeLists[lookup[rem]]) :
			#endif // __FASTLOOKUP__
				&(heapManager->freeLists[Bsearchl( rem, bucketSizes, heapMaster.maxBucketsUsed )]); // binary search

			// The remaining storage may not be bucket size, whereas all other allocations are. Round down to previous
			// bucket size in this case.
			if ( UNLIKELY( freeHead->blockSize > (size_t)rem ) ) freeHead -= 1;
			Heap::Storage * block = (Heap::Storage *)heapManager->bufStart;

			block->header.kind.real.next = freeHead->freeList; // push on stack
			freeHead->freeList = block;
		} // if
	} // if

	heapManager->bufRemaining = increase - size;
	heapManager->bufStart = (char *)newblock + size;
	return newblock;
} // manager_extend


#ifdef __U_STATISTICS__
#define STAT_NAME __counter
#define STAT_PARM , unsigned int STAT_NAME
#define STAT_ARG( name ) , name
#define STAT_0_CNT( counter ) heapManager->stats.counters[counter].calls_0 += 1
#else
#define STAT_NAME
#define STAT_PARM
#define STAT_ARG( name )
#define STAT_0_CNT( counter )
#endif // __U_STATISTICS__

#define BOOT_HEAP_MANAGER() \
  	if ( UNLIKELY( heapManager == (Heap *)1 ) ) { /* new thread ? */ \
		heapManagerCtor(); /* trigger for first heap, singleton */ \
		assert( heapManager ); \
	} /* if */

// NULL_0_ALLOC is disabled because a lot of programs incorrectly check for out of memory by just checking for a NULL
// return from malloc, rather than checking both NULL return and errno == ENOMEM.
//#define __NULL_0_ALLOC__ /* Uncomment to return null address for malloc( 0 ). */

// Do not use '\xfe' for scrubbing because dereferencing an address composed of it causes a SIGSEGV *without* a valid IP
// pointer in the interrupt frame.
#define SCRUB '\xff'									// scrub value
#define SCRUB_SIZE 1024lu								// scrub size, front/back/all of allocation area

__attribute__(( noinline, noclone, section( "text_nopreempt" ) ))
static void * doMalloc( size_t size STAT_PARM ) {
	BOOT_HEAP_MANAGER();

	#ifdef __NULL_0_ALLOC__
	if ( UNLIKELY( size == 0 ) ) {
		STAT_0_CNT( STAT_NAME );
		return nullptr;
	} // if
	#endif // __NULL_0_ALLOC__

	#ifdef __U_DEBUG__
	if ( UNLIKELY( size > ULONG_MAX - sizeof(Heap::Storage) ) ) {
		errno = ENOMEM;
		return nullptr;
	} // if
	#endif // __U_DEBUG__

	Heap::Storage * block;								// pointer to new block of storage

	// Look up size in the size list.

	// The user request must include space for the header allocated along with the storage block and is a multiple of
	// the alignment size.
	size_t tsize = size + sizeof(Heap::Storage);		// total request space needed
	Heap * heap = heapManager;							// optimization, as heapManager is thread_local

	#ifdef __U_STATISTICS__
	if ( UNLIKELY( size == 0 ) ) {						// malloc( 0 ) ?
		heap->stats.counters[STAT_NAME].calls_0 += 1;
	} else {
		heap->stats.counters[STAT_NAME].calls += 1;
		heap->stats.counters[STAT_NAME].request += size;
	} // if
	#endif // __U_STATISTICS__

	uDEBUG( heap->allocUnfreed += size; );

	if ( LIKELY( size < heapMaster.mmapStart ) ) {		// small size => sbrk
		Heap::FreeHeader * freeHead =
			#ifdef __FASTLOOKUP__
			LIKELY( tsize < LookupSizes ) ? &(heap->freeLists[lookup[tsize]]) :
			#endif // __FASTLOOKUP__
			&(heapManager->freeLists[Bsearchl( tsize, bucketSizes, heapMaster.maxBucketsUsed )]); // binary search

		assert( freeHead <= &heap->freeLists[heapMaster.maxBucketsUsed] ); // subscripting error ?
		assert( tsize <= freeHead->blockSize );			// search failure ?

		#ifdef __U_STATISTICS__
		heap->stats.counters[STAT_NAME].alloc += freeHead->blockSize; // total space needed for request
		#endif // __U_STATISTICS__

		// The checking order for freed storage versus bump storage has a performance difference, if there are lots of
		// allocations before frees. The following checks for freed storage first in an attempt to reduce the storage
		// footprint, i.e., starting using freed storage before using all the free block.

		block = freeHead->freeList;						// remove node from stack
		// For reused memory, it is scrubbed in doFree for debug, so no scrubbing on allocation side.
		if ( LIKELY( block != nullptr ) ) {				// free block ?
			// Get storage from the corresponding free list.
			freeHead->freeList = block->header.kind.real.next;

			#ifdef __U_STATISTICS__
			freeHead->reuses += 1;
			#endif // __U_STATISTICS__
		} else {
			// Get storage from free block using bump allocation.
			tsize = freeHead->blockSize;				// optimization, bucket size for request
			ptrdiff_t rem = heapManager->bufRemaining - tsize;
			if ( LIKELY( rem >= 0 ) ) {					// bump storage ?
				heapManager->bufRemaining = rem;
				block = (Heap::Storage *)heapManager->bufStart;
				heapManager->bufStart = (char *)heapManager->bufStart + tsize;
			#ifdef __OWNERSHIP__
				// Race with adding thread, get next time if lose race.
			} else if ( UNLIKELY( freeHead->remoteList ) ) { // returned space ?
				// Get storage by removing entire remote list and chain onto appropriate free list.
				#ifdef __REMOTESPIN__
				freeHead->remoteLock->acquire_( true );
				block = freeHead->remoteList;
				freeHead->remoteList = nullptr;
				freeHead->remoteLock->release_( true );
				#else
				block = uFetchAssign( freeHead->remoteList, (decltype(freeHead->remoteList))nullptr ); // must be same type
				#endif // __REMOTESPIN__

				assert( block );
				#ifdef __U_STATISTICS__
				heap->stats.remote_pulls += 1;
				#endif // __U_STATISTICS__

				// OK TO BE PREEMPTED HERE AS heapManager IS NO LONGER ACCESSED.

				freeHead->freeList = block->header.kind.real.next; // merge remoteList into freeHead
			#endif // __OWNERSHIP__
			} else {
				// Get storage from a *new* free block using bump alocation.
				uKernelModule::uKernelModuleData::disableInterrupts();
				block = (Heap::Storage *)manager_extend( tsize ); // mutual exclusion on call
				uKernelModule::uKernelModuleData::enableInterruptsNoRF();

				// OK TO BE PREEMPTED HERE AS heapManager IS NO LONGER ACCESSED.

				// For new memory, scrub so subsequent uninitialized usages might fail. Only scrub the first SCRUB_SIZE bytes.
				uDEBUG( memset( block->data, SCRUB, Min( SCRUB_SIZE, tsize - sizeof(Heap::Storage) ) ); );
			} // if

			#ifdef __U_STATISTICS__
			freeHead->allocations += 1;
			#endif // __U_STATISTICS__
		} // if

		block->header.kind.real.home = freeHead;		// pointer back to free list of apropriate size
	} else {											// large size => mmap
		#ifdef __DEBUG__
		// Recheck because of minimum allocation size (page size).
		if ( UNLIKELY( size > ULONG_MAX - heapMaster.pageSize ) ) {
			errno = ENOMEM;
			return nullptr;
		} // if
		#endif // __DEBUG__

		tsize = uCeiling( tsize, heapMaster.pageSize );	// must be multiple of page size

		#ifdef __U_STATISTICS__
		heap->stats.counters[STAT_NAME].alloc += tsize;
		heap->stats.mmap_calls += 1;
		heap->stats.mmap_storage_request += size;
		heap->stats.mmap_storage_alloc += tsize;
		#endif // __U_STATISTICS__

		uKernelModule::uKernelModuleData::disableInterrupts();
		block = (Heap::Storage *)::mmap( 0, tsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
		uKernelModule::uKernelModuleData::enableInterruptsNoRF();

		// OK TO BE PREEMPTED HERE AS heapManager IS NO LONGER ACCESSED.

		if ( UNLIKELY( block == MAP_FAILED ) ) {		// failed ?
			if ( errno == ENOMEM ) { return nullptr; }	// no memory
			// Do not call strerror( errno ) as it may call malloc.
			abort( "attempt to allocate large object (> %zu) of size %zu bytes and mmap failed with errno %d.",
				   size, heapMaster.mmapStart, errno );
		} // if
		block->header.kind.real.blockSize = MarkMmappedBit( tsize ); // storage size for munmap

		// For new memory, scrub so subsequent uninitialized usages might fail. Only scrub the first SCRUB_SIZE bytes.
		// The rest of the storage set to 0 by mmap.
		uDEBUG( memset( block->data, SCRUB, Min( SCRUB_SIZE, tsize - sizeof(Heap::Storage) ) ); );
	} // if

	block->header.kind.real.size = size;				// store allocation size
	void * addr = &(block->data);						// adjust off header to user bytes
	assert( ((uintptr_t)addr & (uAlign() - 1)) == 0 );	// minimum alignment ?

	#ifdef __U_DEBUG__
	if ( UPP::uHeapControl::traceHeap() ) {
		enum { BufferSize = 64 };
		char helpText[BufferSize];
		int len = snprintf( helpText, BufferSize, "%p = Malloc( %zu ) (allocated %zu)\n", addr, size, tsize );
		uDebugWrite( STDERR_FILENO, helpText, len );	// print debug/nodebug
	} // if
	#endif // __U_DEBUG__

	return addr;
} // doMalloc


__attribute__(( noinline, noclone, section( "text_nopreempt" ) ))
static void doFree( void * addr ) {
	#if defined( __STATISTICS__ ) || defined( __DEBUG__ ) || ! defined( __OWNERSHIP__ )
	// A thread can run without a heap, and hence, have an uninitialized heapManager. For example, in the ownership
	// program, the consumer thread does not allocate storage, it only frees storage back to the owning producer
	// thread. Hence, the consumer never calls doMalloc to trigger heap creation for its thread. However, this situation
	// is a problem for statistics, the consumer needs a heap to gather statistics. Hence, the special case to trigger a
	// heap if there is none.
	BOOT_HEAP_MANAGER();								// singlton
	#endif // __STATISTICS__ || __DEBUG__

	// At this point heapManager can be null because there is a thread_local deallocation *after* the destructor for the
	// thread termination is run. The code below is prepared to handle this case and uses the master heap for statistics.

	assert( addr );										// addr == nullptr handled in free
	Heap * heap = heapManager;							// optimization, as heapManager is thread_local

	Heap::Storage::Header * header;
	Heap::FreeHeader * freeHead;
	size_t tsize, alignment;

	bool mapped = headers( "free", addr, header, freeHead, tsize, alignment );
	#if defined( __U_STATISTICS__ ) || defined( __U_DEBUG__ )
	size_t size = header->kind.real.size;				// optimization
	#endif // __U_STATISTICS__ || __U_DEBUG__

	#ifdef __U_STATISTICS__
	#ifndef __NULL_0_ALLOC__
	if ( UNLIKELY( size == 0 ) )						// malloc( 0 ) ?
		heap->stats.free_null_0_calls += 1;
	else
	#endif // __NULL_0_ALLOC__
		heap->stats.free_calls += 1;					// count free amd implicit frees from resize/realloc
	heap->stats.free_storage_request += size;
	heap->stats.free_storage_alloc += tsize;
	#endif // __U_STATISTICS__

	uDEBUG( heap->allocUnfreed -= size; );

	if ( LIKELY( ! mapped ) ) {							// sbrk ?
		assert( freeHead );
		#ifdef __U_DEBUG__
		// memset is NOT always inlined!
		uKernelModule::uKernelModuleData::disableInterrupts();
		// Scrub old memory so subsequent usages might fail. Only scrub the first/last SCRUB_SIZE bytes.
		char * data = ((Heap::Storage *)header)->data;	// data address
		size_t dsize = tsize - sizeof(Heap::Storage);	// data size
		if ( dsize <= SCRUB_SIZE * 2 ) {
			memset( data, SCRUB, dsize );				// scrub all
		} else {
			memset( data, SCRUB, SCRUB_SIZE );			// scrub front
			memset( data + dsize - SCRUB_SIZE, SCRUB, SCRUB_SIZE ); // scrub back
		} // if
		uKernelModule::uKernelModuleData::enableInterruptsNoRF();
		#endif // __U_DEBUG__

		#ifdef __OWNERSHIP__
		if ( LIKELY( heap == freeHead->homeManager ) ) { // belongs to this thread
			header->kind.real.next = freeHead->freeList; // push on stack
			freeHead->freeList = (Heap::Storage *)header;
		} else {										// return to thread owner
			#ifdef __REMOTESPIN__
			freeHead->remoteLock->acquire_( true );
			header->kind.real.next = freeHead->remoteList; // push entire remote list to bucket
			freeHead->remoteList = (Heap::Storage *)header;
			freeHead->remoteLock->release_( true );
			#else										// lock free
			header->kind.real.next = freeHead->remoteList; // link new node to top node
			// CAS resets header->kind.real.next = freeHead->remoteList on failure
			while ( ! uCompareAssignValue( freeHead->remoteList, header->kind.real.next, (Heap::Storage *)header ) ) uPause();
			#endif // __REMOTESPIN__

			#ifdef __U_STATISTICS__
			heap->stats.remote_pushes += 1;
			heap->stats.remote_storage_request += size;
			heap->stats.remote_storage_alloc += tsize;
			#endif // __U_STATISTICS__
		} // if

		#else											// no OWNERSHIP

		assert( heap );

		// kind.real.home is address in owner thread's freeLists, so compute the equivalent position in this thread's freeList.
		freeHead = &heap->freeLists[ClearStickyBits( header->kind.real.home ) - &freeHead->homeManager->freeLists[0]];
		header->kind.real.next = freeHead->freeList;	// push on stack
		freeHead->freeList = (Heap::Storage *)header;
		#endif // __OWNERSHIP__
	} else {											// mmapped
		#ifdef __U_STATISTICS__
		heap->stats.munmap_calls += 1;
		heap->stats.munmap_storage_request += size;
		heap->stats.munmap_storage_alloc += tsize;
		#endif // __U_STATISTICS__

		// OK TO BE PREEMPTED HERE AS heapManager IS NO LONGER ACCESSED.

		if ( UNLIKELY( munmap( header, tsize ) == -1 ) ) {
			// Do not call strerror( errno ) as it may call malloc.
			abort( "attempt to deallocate large object %p and munmap failed with errno %d.\n"
				   "Possible cause is invalid delete pointer: either not allocated or with corrupt header.",
				   addr, errno );
		} // if
	} // if

	#ifdef __U_DEBUG__
	if ( UPP::uHeapControl::traceHeap() ) {
		char helpText[64];
		int len = snprintf( helpText, sizeof(helpText), "Free( %p ) size %zu allocated %zu\n", addr, size, tsize );
		uDebugWrite( STDERR_FILENO, helpText, len );	// print debug/nodebug
	} // if
	#endif // __U_DEBUG__
} // doFree


// A preemption can change heapManager so statistic updates are associated with the wrong heap. However, all the heap
// statistics are normally aggregated (see malloc_stats), so data in the wrong heap is always counted. Only, for
// heap_stats can potential incorrect statistics be seen, but calling heap_stats is unusual.

// #ifdef __U_STATISTICS__
// __attribute__(( noinline, noclone, section( "text_nopreempt" ) ))
// static void incCalls( size_t statName ) {
// 	heapManager->stats.counters[statName].calls += 1;
// } // incCalls

// __attribute__(( noinline, noclone, section( "text_nopreempt" ) ))
// static void incZeroCalls( size_t statName ) {
// 	heapManager->stats.counters[statName].calls_0 += 1;
// } // incZeroCalls
// #endif // __U_STATISTICS__

// #ifdef __U_DEBUG__
// __attribute__(( noinline, noclone, section( "text_nopreempt" ) ))
// static void incUnfreed( ptrdiff_t offset ) {
// 	heapManager->allocUnfreed += offset;
// } // incUnfreed
// #endif // __U_DEBUG__


static inline __attribute__((always_inline)) void * memalignNoStats( size_t alignment, size_t size STAT_PARM ) {
	checkAlign( alignment );							// check alignment

	// if alignment <= default alignment or size == 0, do normal malloc as two headers are unnecessary
  if ( UNLIKELY( alignment <= uAlign() || size == 0 ) ) return doMalloc( size STAT_ARG( STAT_NAME ) );

	// Allocate enough storage to guarantee an address on the alignment boundary, and sufficient space before it for
	// administrative storage. NOTE, WHILE THERE ARE 2 HEADERS, THE FIRST ONE IS IMPLICITLY CREATED BY DOMALLOC.
	//      .-------------v-----------------v----------------v----------,
	//      | Real Header | ... padding ... |   Fake Header  | data ... |
	//      `-------------^-----------------^-+--------------^----------'
	//      |<--------------------------------' offset/align |<-- alignment boundary

	// subtract uAlign() because it is already the minimum alignment
	// add sizeof(Heap::Storage) for fake header
	size_t offset = alignment - uAlign() + sizeof(Heap::Storage);
	char * addr = (char *)doMalloc( size + offset STAT_ARG( STAT_NAME ) );

  if ( UNLIKELY( addr == nullptr ) ) return nullptr;	// stop further processing if nullptr is returned

	// address in the block of the "next" alignment address
	char * user = (char *)uCeiling( (uintptr_t)(addr + sizeof(Heap::Storage)), alignment );

	// address of header from malloc
	Heap::Storage::Header * realHeader = HeaderAddr( addr );
	realHeader->kind.real.size = size;					// correct size to eliminate above alignment offset
	uDEBUG( heapManager->allocUnfreed -= offset; );		// adjustment off the offset from call to doMalloc

	// address of fake header *before* the alignment location
	Heap::Storage::Header * fakeHeader = HeaderAddr( user );

	// SKULLDUGGERY: insert the offset to the start of the actual storage block and remember alignment
	fakeHeader->kind.fake.offset = (char *)fakeHeader - (char *)realHeader;
	// SKULLDUGGERY: odd alignment implies fake header
	fakeHeader->kind.fake.alignment = MarkAlignmentBit( alignment );

	return user;
} // memalignNoStats

// Operators new and new [] call malloc; delete calls free


//####################### Memory Allocation Routines ####################


// Realloc dilemma:
//
// 1. If realloc fails (ENOMEM), the original block is left untouched; it is not freed or moved, and nullptr is returned.
//
// 2. However, the common realloc pattern, p = realloc( p, size ), leaks memory in this case as the only address to the
//    old storage is overwritten with nullptr.
//
// The only way to solve this dilemma is by adopting the posix_memalign strategy of returning two values: return code
// and new memory address. Alternatively, C++ has two outcomes: return memory address or raise an exception from "new"
// when out of memory.

extern "C" {
	// Allocates size bytes and returns a pointer to the allocated memory.  The contents are undefined. If size is 0,
	// then malloc() returns a unique pointer value that can later be successfully passed to free().
	void * malloc( size_t size ) __THROW {
		return doMalloc( size STAT_ARG( HeapStatistics::MALLOC ) );
	} // malloc


	// Same as malloc() except size bytes is an array of dimension elements each of elemSize bytes.
	void * aalloc( size_t dimension, size_t elemSize ) __THROW {
		return doMalloc( dimension * elemSize STAT_ARG( HeapStatistics::AALLOC ) );
	} // aalloc


	// Same as aalloc() with memory set to zero.
	void * calloc( size_t dimension, size_t elemSize ) __THROW {
		size_t size = dimension * elemSize;
		char * addr = (char *)doMalloc( size STAT_ARG( HeapStatistics::CALLOC ) );

	  if ( UNLIKELY( addr == nullptr ) ) return nullptr; // stop further processing if nullptr is returned

		Heap::Storage::Header * header = HeaderAddr( addr ); // optimization

		#ifndef __U_DEBUG__
		// Mapped storage is zero filled, but in debug mode mapped memory is scrubbed in doMalloc, so it has to be reset to zero.
		if ( LIKELY( ! MmappedBit( header ) ) )
		#endif // __U_DEBUG__
			// <-------0000000000000000000000000000UUUUUUUUUUUUUUUUUUUUUUUUU> bsize (bucket size) U => undefined
			// `-header`-addr                      `-size
			memset( addr, '\0', size );					// set to zeros

		MarkZeroFilledBit( header );					// mark as zero fill
		return addr;
	} // calloc


	// Change the size of the memory block pointed to by oaddr to size bytes. The contents are undefined.  If oaddr is
	// nullptr, then the call is equivalent to malloc(size), for all values of size; if size is equal to zero, and oaddr
	// is not nullptr, then the call is equivalent to free(oaddr). Unless oaddr is nullptr, it must have been returned
	// by an earlier call to malloc(), alloc(), calloc() or realloc(). If the original area must be moved, a free(oaddr)
	// is always done, even if the allocation of new storage fails.
	void * resize( void * oaddr, size_t size ) __THROW {
	  if ( UNLIKELY( oaddr == nullptr ) ) {				// => malloc( size )
			return doMalloc( size STAT_ARG( HeapStatistics::RESIZE ) );
		} // if

	  if ( UNLIKELY( size == 0 ) ) {
			STAT_0_CNT( HeapStatistics::RESIZE );
			doFree( oaddr );
			return nullptr;
		} // if

		Heap::Storage::Header * header;
		Heap::FreeHeader * freeHead;
		size_t bsize, oalignment;
		headers( "resize", oaddr, header, freeHead, bsize, oalignment );

		size_t odsize = DataStorage( bsize, oaddr, header ); // data storage available in bucket
		// same size, DO NOT PRESERVE STICKY PROPERTIES.
		if ( oalignment == uAlign() && size <= odsize && odsize <= size * 2 ) { // allow 50% wasted storage for smaller size
			ClearZeroFillBit( header );					// no alignment and turn off 0 fill
			uDEBUG(	heapManager->allocUnfreed += size - header->kind.real.size; ); // adjustment off the size difference
			header->kind.real.size = size;				// reset allocation size
			#ifdef __U_STATISTICS__
			heapManager->stats.resize_calls += 1;
			#endif // __U_STATISTICS__
			return oaddr;
		} // if

		// change size, DO NOT PRESERVE STICKY PROPERTIES.
		doFree( oaddr );								// free original storage
		return doMalloc( size STAT_ARG( HeapStatistics::RESIZE ) ); // create new area
	} // resize


	// Same as resize() except the new allocation size is large enough for an array of nelem elements of size elemSize.
	void * resizearray( void * oaddr, size_t dimension, size_t elemSize ) __THROW {
		return resize( oaddr, dimension * elemSize );
	} // resizearray


	// Same as resize() but the contents are unchanged in the range from the start of the region up to the minimum of
	// the old and new sizes.
	void * realloc( void * oaddr, size_t size ) __THROW {
	  if ( UNLIKELY( oaddr == nullptr ) ) {				// => malloc( size )
		  return doMalloc( size STAT_ARG( HeapStatistics::REALLOC ) );
		} // if

	  if ( UNLIKELY( size == 0 ) ) {
			STAT_0_CNT( HeapStatistics::REALLOC );
			doFree( oaddr );
			return nullptr;
		} // if

		Heap::Storage::Header * header;
		Heap::FreeHeader * freeHead;
		size_t bsize, oalignment;
		headers( "realloc", oaddr, header, freeHead, bsize, oalignment );

		size_t odsize = DataStorage( bsize, oaddr, header ); // data storage available in bucket
		size_t osize = header->kind.real.size;			// old allocation size
		bool ozfill = ZeroFillBit( header );			// old allocation zero filled
	  if ( UNLIKELY( size <= odsize ) && odsize <= size * 2 ) { // allow up to 50% wasted storage
			uDEBUG(	heapManager->allocUnfreed += size - header->kind.real.size; ); // adjustment off the size difference
			header->kind.real.size = size;				// reset allocation size
	  		if ( UNLIKELY( ozfill ) && size > osize ) {	// previous request zero fill and larger ?
				#ifdef __U_STATISTICS__
				heapManager->stats.realloc_0_fill += 1;
				#endif // __U_STATISTICS__
	  			memset( (char *)oaddr + osize, '\0', size - osize ); // initialize added storage
	  		} // if
			#ifdef __U_STATISTICS__
			heapManager->stats.realloc_calls += 1;
			heapManager->stats.realloc_smaller += 1;
			#endif // __U_STATISTICS__
			return oaddr;
		} // if

		// change size and copy old content to new storage

		#ifdef __U_STATISTICS__
		heapManager->stats.realloc_copy += 1;
		#endif // __U_STATISTICS__

		void * naddr;
		if ( LIKELY( oalignment <= uAlign() ) ) {		// previous request not aligned ?
			naddr = doMalloc( size STAT_ARG( HeapStatistics::REALLOC ) ); // create new area
		} else {
			#ifdef __U_STATISTICS__
			heapManager->stats.realloc_align += 1;
			#endif // __U_STATISTICS__
			naddr = memalignNoStats( oalignment, size STAT_ARG( HeapStatistics::REALLOC ) ); // create new aligned area
		} // if

	if ( UNLIKELY( naddr == nullptr ) ) return nullptr;	// stop further processing if nullptr is returned

		header = HeaderAddr( naddr );					// new header
		size_t alignment;
		fakeHeader( header, alignment );				// could have a fake header

		// To preserve prior fill, the entire bucket must be copied versus the size.
		memcpy( naddr, oaddr, Min( osize, size ) );		// copy bytes
		doFree( oaddr );								// free previous storage

		if ( UNLIKELY( ozfill ) ) {						// previous request zero fill ?
			MarkZeroFilledBit( header );				// mark new request as zero filled
			if ( size > osize ) {						// previous request larger ?
				#ifdef __U_STATISTICS__
				heapManager->stats.realloc_0_fill += 1;
				#endif // __U_STATISTICS__
				memset( (char *)naddr + osize, '\0', size - osize ); // initialize added storage
			} // if
		} // if
		return naddr;
	} // realloc


	// Same as realloc() except the new allocation size is large enough for an array of nelem elements of size elemSize.
	void * reallocarray( void * oaddr, size_t dimension, size_t elemSize ) __THROW {
		return realloc( oaddr, dimension * elemSize );
	} // reallocarray


	// Same as realloc() but prevents the p = realloc( p, size ) memory leak when ENOMEM returns nullptr.
	int posix_realloc( void ** oaddrp, size_t size ) __THROW {
		void * ret = realloc( *oaddrp, size );
		if ( ret == nullptr && errno == ENOMEM ) return ENOMEM;
		*oaddrp = ret;
		return 0;
	} // posix_realloc


	// Same as posix_realloc() except the new allocation size is large enough for an array of nelem elements of size elemSize.
	int posix_reallocarray( void ** oaddrp, size_t dimension, size_t elemSize ) __THROW {
		return posix_realloc( oaddrp, dimension * elemSize );
	} // posix_realloc


	// Below, same as their counterparts above but with alignment.

	void * aligned_resize( void * oaddr, size_t nalignment, size_t size ) __THROW {
	  if ( UNLIKELY( oaddr == nullptr ) ) {				// => malloc( size )
			return memalignNoStats( nalignment, size STAT_ARG( HeapStatistics::RESIZE ) );
		} // if

	  if ( UNLIKELY( size == 0 ) ) {
			STAT_0_CNT( HeapStatistics::RESIZE );
			doFree( oaddr );
			return nullptr;
		} // if

		// Attempt to reuse existing alignment.
		Heap::Storage::Header * header = HeaderAddr( oaddr );
		bool isFakeHeader = AlignmentBit( header );		// old fake header ?
		size_t oalignment;

		if ( UNLIKELY( isFakeHeader ) ) {
			checkAlign( nalignment );					// check alignment
			oalignment = ClearAlignmentBit( header );	// old alignment

			if ( UNLIKELY( (uintptr_t)oaddr % nalignment == 0 // lucky match ?
				 && ( oalignment <= nalignment			// going down
					  || (oalignment >= nalignment && oalignment <= 256) ) // little alignment storage wasted ?
				) ) {
				HeaderAddr( oaddr )->kind.fake.alignment = MarkAlignmentBit( nalignment ); // update alignment (could be the same)
				Heap::FreeHeader * freeHead;
				size_t bsize;
				headers( "aligned_resize", oaddr, header, freeHead, bsize, oalignment );
				size_t odsize = DataStorage( bsize, oaddr, header ); // data storage available in bucket

				if ( size <= odsize && odsize <= size * 2 ) { // allow 50% wasted data storage
					HeaderAddr( oaddr )->kind.fake.alignment = MarkAlignmentBit( nalignment ); // update alignment (could be the same)

					ClearZeroFillBit( header );			// turn off 0 fill
					#ifdef __U_DEBUG__
					heapManager->allocUnfreed += size - header->kind.real.size; // adjustment off the size difference
					#endif // __U_DEBUG__
					header->kind.real.size = size;		// reset allocation size
					#ifdef __U_STATISTICS__
					heapManager->stats.resize_calls += 1;
					#endif // __U_STATISTICS__
					return oaddr;
				} // if
			} // if
		} else if ( ! isFakeHeader						// old real header (aligned on libAlign) ?
					&& nalignment == uAlign() ) {		// new alignment also on libAlign => no fake header needed
			return resize( oaddr, size );				// duplicate special case checks
		} // if

		// change size, DO NOT PRESERVE STICKY PROPERTIES.
		doFree( oaddr );								// free original storage
		return memalignNoStats( nalignment, size STAT_ARG( HeapStatistics::RESIZE ) ); // create new aligned area
	} // aligned_resize


	void * aligned_resizearray( void * oaddr, size_t nalignment, size_t dimension, size_t elemSize ) __THROW {
		return aligned_resize( oaddr, nalignment, dimension * elemSize );
	} // aligned_resizearray


	void * aligned_realloc( void * oaddr, size_t nalignment, size_t size ) __THROW {
	  if ( UNLIKELY( oaddr == nullptr ) ) {				// => malloc( size )
			return memalignNoStats( nalignment, size STAT_ARG( HeapStatistics::REALLOC ) );
		} // if

	  if ( UNLIKELY( size == 0 ) ) {
			STAT_0_CNT( HeapStatistics::REALLOC );
			doFree( oaddr );
			return nullptr;
		} // if

		// Attempt to reuse existing alignment.
		Heap::Storage::Header * header = HeaderAddr( oaddr );
		bool isFakeHeader = AlignmentBit( header );		// old fake header ?
		size_t oalignment;

		if ( UNLIKELY( isFakeHeader ) ) {
			checkAlign( nalignment );					// check alignment
			oalignment = ClearAlignmentBit( header );	// old alignment
			if ( UNLIKELY( (uintptr_t)oaddr % nalignment == 0 // lucky match ?
				 && ( oalignment <= nalignment			// going down
					  || (oalignment >= nalignment && oalignment <= 256) ) // little alignment storage wasted ?
				) ) {
				HeaderAddr( oaddr )->kind.fake.alignment = MarkAlignmentBit( nalignment ); // update alignment (could be the same)
				return realloc( oaddr, size );			// duplicate special case checks
			} // if
		} else if ( ! isFakeHeader						// old real header (aligned on libAlign) ?
					&& nalignment == uAlign() ) {		// new alignment also on libAlign => no fake header needed
			return realloc( oaddr, size );				// duplicate special case checks
		} // if

		Heap::FreeHeader * freeHead;
		size_t bsize;
		headers( "aligned_realloc", oaddr, header, freeHead, bsize, oalignment );

		// change size and copy old content to new storage

		size_t osize = header->kind.real.size;			// old allocation size
		bool ozfill = ZeroFillBit( header );			// old allocation zero filled

		void * naddr = memalignNoStats( nalignment, size STAT_ARG( HeapStatistics::REALLOC ) ); // create new aligned area

	if ( UNLIKELY( naddr == nullptr ) ) return nullptr;	// stop further processing if nullptr is returned

		header = HeaderAddr( naddr );					// new header
		size_t alignment;
		fakeHeader( header, alignment );				// could have a fake header

		memcpy( naddr, oaddr, Min( osize, size ) );		// copy bytes
		doFree( oaddr );								// free previous storage

		if ( UNLIKELY( ozfill ) ) {						// previous request zero fill ?
			MarkZeroFilledBit( header );				// mark new request as zero filled
			if ( size > osize ) {						// previous request larger ?
				memset( (char *)naddr + osize, '\0', size - osize ); // initialize added storage
			} // if
		} // if
		return naddr;
	} // aligned_realloc


	void * aligned_reallocarray( void * oaddr, size_t nalignment, size_t dimension, size_t elemSize ) __THROW {
		return aligned_realloc( oaddr, nalignment, dimension * elemSize );
	} // aligned_reallocarray


	// Same as aligned_realloc() but prevents the p = realloc( p, size ) memory leak when ENOMEM returns nullptr.
	int posix_aligned_realloc( void ** oaddrp, size_t nalignment, size_t size ) __THROW {
		void * ret = aligned_realloc( *oaddrp, nalignment, size );
		if ( ret == nullptr && errno == ENOMEM ) return ENOMEM;
		*oaddrp = ret;
		return 0;
	} // posix_aligned_realloc


	// Same as posix_aligned_realloc() except the new allocation size is large enough for an array of nelem elements of size elemSize.
	int posix_aligned_reallocarray( void ** oaddrp, size_t nalignment, size_t dimension, size_t elemSize ) __THROW {
		return posix_aligned_realloc( oaddrp, nalignment, dimension * elemSize );
	} // posix_aligned_realloc


	// Same as malloc() except the memory address is a multiple of alignment, which must be a power of two. (obsolete)
	void * memalign( size_t alignment, size_t size ) __THROW {
		return memalignNoStats( alignment, size STAT_ARG( HeapStatistics::MEMALIGN ) );
	} // memalign


	// Same as aalloc() with memory alignment.
	void * amemalign( size_t alignment, size_t dimension, size_t elemSize ) __THROW {
		return memalignNoStats( alignment, dimension * elemSize STAT_ARG( HeapStatistics::AMEMALIGN ) );
	} // amemalign


	// Same as calloc() with memory alignment.
	void * cmemalign( size_t alignment, size_t dimension, size_t elemSize ) __THROW {
		size_t size = dimension * elemSize;
		char * addr = (char *)memalignNoStats( alignment, size STAT_ARG( HeapStatistics::CMEMALIGN ) );

	  if ( UNLIKELY( addr == nullptr ) ) return nullptr; // stop further processing if nullptr is returned

		Heap::Storage::Header * header = HeaderAddr( addr ); // optimization
		fakeHeader( header, alignment );				// must have a fake header

		#ifndef __U_DEBUG__
		// Mapped storage is zero filled, but in debug mode mapped memory is scrubbed in doMalloc, so it has to be reset to zero.
		if ( LIKELY( ! MmappedBit( header ) ) )
		#endif // __U_DEBUG__
			// <-------0000000000000000000000000000UUUUUUUUUUUUUUUUUUUUUUUUU> bsize (bucket size) U => undefined
			// `-header`-addr                      `-size
			memset( addr, '\0', size );					// set to zeros

		MarkZeroFilledBit( header );					// mark as zero fill
		return addr;
	} // cmemalign


	// Same as memalign(), but ISO/IEC 2011 C11 Section 7.22.2 states: the value of size shall be an integral multiple
	// of alignment. This requirement is universally ignored.
	void * aligned_alloc( size_t alignment, size_t size ) __THROW {
		return memalign( alignment, size );
	} // aligned_alloc


	// Allocates size bytes and places the address of the allocated memory in *memptr. The address of the allocated
	// memory shall be a multiple of alignment, which must be a power of two and a multiple of sizeof(void *). If size
	// is 0, then posix_memalign() returns either nullptr, or a unique pointer value that can later be successfully
	// passed to free(3).
	int posix_memalign( void ** memptr, size_t alignment, size_t size ) __THROW {
	  if ( UNLIKELY( alignment < uAlign() || ! uPow2( alignment ) ) ) return EINVAL; // check alignment
		void * ret = memalign( alignment, size );
		if ( ret == nullptr ) return ENOMEM;
		*memptr = ret;									// only update on success
		return 0;
	} // posix_memalign


	// Allocates size bytes and returns a pointer to the allocated memory. The memory address shall be a multiple of the
	// page size.  It is equivalent to memalign(sysconf(_SC_PAGESIZE),size).
	void * valloc( size_t size ) __THROW {
		return memalign( heapMaster.pageSize, size );
	} // valloc


	// Same as valloc but rounds size to multiple of page size. It is equivalent to valloc( ceiling( size,
	// sysconf(_SC_PAGESIZE) ) ).
	void * pvalloc( size_t size ) __THROW {
		return memalign( heapMaster.pageSize, uCeiling( size, heapMaster.pageSize ) );
	} // pvalloc


	// Frees the memory space pointed to by ptr, which must have been returned by a previous call to malloc(), calloc()
	// or realloc().  Otherwise, or if free(ptr) has already been called before, undefined behaviour occurs. If ptr is
	// nullptr, no operation is performed.
	void free( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) {				// special case
			#ifdef __U_STATISTICS__
			heapManager->stats.free_null_0_calls += 1;
			#endif // __U_STATISTICS__
			return;
		} // if

		doFree( addr );									// handles heapManager == nullptr
	} // free


	// Sets the amount (bytes) to extend the heap when there is insufficent free storage to service an allocation.
	__attribute__((weak)) size_t malloc_extend( void ) { return __DEFAULT_HEAP_EXTEND__; }

	// Sets the crossover point between allocations occuring in the sbrk area or separately mmapped.
	__attribute__((weak)) size_t malloc_mmap_start( void ) { return __DEFAULT_MMAP_START__; }

	// Amount subtracted to adjust for unfreed program storage (debug only).
	__attribute__((weak)) size_t malloc_unfreed( void ) { return __DEFAULT_HEAP_UNFREED__; }


	// Returns original total allocation size (not bucket size) => array size is dimension * sizeof(T).
	size_t malloc_size( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) return 0;		// null allocation has zero size
		Heap::Storage::Header * header = HeaderAddr( addr );
		if ( UNLIKELY( AlignmentBit( header ) ) ) {		// fake header ?
			header = RealHeader( header );				// backup from fake to real header
		} // if
		return header->kind.real.size;
	} // malloc_size


	// Returns the alignment of an allocation.
	size_t malloc_alignment( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) return uAlign(); // minimum alignment
		Heap::Storage::Header * header = HeaderAddr( addr );
		if ( UNLIKELY( AlignmentBit( header ) ) ) {		// fake header ?
			return ClearAlignmentBit( header );			// clear flag from value
		} else {
			return uAlign();							// minimum alignment
		} // if
	} // malloc_alignment


	// Returns true if the allocation is zero filled, e.g., allocated by calloc().
	bool malloc_zero_fill( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) return false;	// null allocation is not zero fill
		Heap::Storage::Header * header = HeaderAddr( addr );
		if ( UNLIKELY( AlignmentBit( header ) ) ) {		// fake header ?
			header = RealHeader( header );				// backup from fake to real header
		} // if
		return ZeroFillBit( header );					// zero filled ?
	} // malloc_zero_fill


	bool malloc_remote( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) return false;	// null allocation is not zero fill
		Heap::Storage::Header * header = HeaderAddr( addr );
		if ( UNLIKELY( AlignmentBit( header ) ) ) {		// fake header ?
			header = RealHeader( header );				// backup from fake to real header
		} // if
		return heapManager == (ClearStickyBits( header->kind.real.home ))->homeManager;
	} // malloc_remote


	// Returns the number of usable bytes in the block pointed to by ptr, a pointer to a block of memory allocated by
	// malloc or a related function. Returned size is >= allocation size (bucket size).
	size_t malloc_usable_size( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) return 0;		// null allocation has zero size
		Heap::Storage::Header * header;
		Heap::FreeHeader * freeHead;
		size_t bsize, alignment;

		headers( "malloc_usable_size", addr, header, freeHead, bsize, alignment );
		return DataStorage( bsize, addr, header );		// data storage in bucket
	} // malloc_usable_size


	// Prints (on default standard error) statistics about memory allocated by malloc and related functions.
	void malloc_stats( void ) __THROW {
		#ifdef __U_STATISTICS__
		HeapStatistics stats;
		HeapStatisticsCtor( stats );
		printStats( collectStats( stats ) );			// file might be closed
		#else
		#define MALLOC_STATS_MSG "malloc_stats statistics disabled.\n"
		uDebugWrite( STDERR_FILENO, MALLOC_STATS_MSG, sizeof( MALLOC_STATS_MSG ) - 1 /* size includes '\0' */ ); // file might be closed
		#endif // __U_STATISTICS__
	} // malloc_stats

	// Zero the heap master and all active thread heaps.
	void malloc_stats_clear( void ) __THROW {
		#ifdef __U_STATISTICS__
		clearStats();
		#endif // __U_STATISTICS__
	} // malloc_stats_clear

	// Set file descriptor where malloc_stats/malloc_info writes statistics.
	int malloc_stats_fd( int fd __attribute__(( unused )) ) __THROW {
		#ifdef __U_STATISTICS__
		int temp = heapMaster.stats_fd;
		heapMaster.stats_fd = fd;
		return temp;
		#else
		return -1;										// unsupported
		#endif // __U_STATISTICS__
	} // malloc_stats_fd

	void heap_stats( void ) __THROW {
		#ifdef __U_STATISTICS__
		char title[32];
		snprintf( title, 32, " (%p)", heapManager );	// always puts a null terminator
		printStats( heapManager->stats, title );		// file might be closed
		#else
		#define MALLOC_STATS_MSG "malloc_stats statistics disabled.\n"
		uDebugWrite( STDERR_FILENO, MALLOC_STATS_MSG, sizeof( MALLOC_STATS_MSG ) - 1 /* size includes '\0' */ ); // file might be closed
		#endif // __U_STATISTICS__
	} // heap_stats


	// Prints an XML string that describes the current state of the memory-allocation implementation in the caller.
	// The string is printed on the file stream.  The exported string includes information about all arenas (see
	// malloc).
	int malloc_info( int options, FILE * stream __attribute__(( unused )) ) {
	  if ( options != 0 ) { errno = EINVAL; return -1; }
		#ifdef __U_STATISTICS__
		HeapStatistics stats;
		HeapStatisticsCtor( stats );
		return printStatsXML( collectStats( stats ), stream ); // returns bytes written or -1
		#else
		return 0;										// unsupported
		#endif // __U_STATISTICS__
	} // malloc_info


	// Adjusts parameters that control the behaviour of the memory-allocation functions (see malloc). The param argument
	// specifies the parameter to be modified, and value specifies the new value for that parameter.
	int mallopt( int option, int value ) __THROW {
	  if ( value < 0 ) return 0;
		switch( option ) {
		  case M_TOP_PAD:
			heapMaster.sbrkExtend = uCeiling( value, heapMaster.pageSize );
			return 1;
		  case M_MMAP_THRESHOLD:
			if ( setMmapStart( value ) ) return 1;
			break;
		} // switch
		return 0;										// error, unsupported
	} // mallopt


	// Attempt to release free memory at the top of the heap (by calling sbrk with a suitable argument).
	int malloc_trim( size_t ) __THROW {
		return 0;										// => impossible to release memory
	} // malloc_trim


	// Records the current state of all malloc internal bookkeeping variables (but not the actual contents of the heap
	// or the state of malloc_hook functions pointers).  The state is recorded in a system-dependent opaque data
	// structure dynamically allocated via malloc, and a pointer to that data structure is returned as the function
	// result.  (The caller must free this memory.)
	void * malloc_get_state( void ) __THROW {
		return nullptr;									// unsupported
	} // malloc_get_state


	// Restores the state of all malloc internal bookkeeping variables to the values recorded in the opaque data
	// structure pointed to by state.
	int malloc_set_state( void * ) __THROW {
		return 0;										// unsupported
	} // malloc_set_state
} // extern "C"

// Local Variables: //
// tab-width: 4 //
// compile-command: "make install" //
// End: //
