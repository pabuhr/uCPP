//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uHeapLmmm.cc -- Lean Mean Malloc Machine - a runtime configurable replacement
//                 for malloc.
// 
// Author           : Peter A. Buhr
// Created On       : Sat Nov 11 16:07:20 1988
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Dec 16 12:28:23 2020
// Update Count     : 1718
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
#include <uHeapLmmm.h>
#include <uAlign.h>
#ifdef __U_PROFILER__
#include <uProfiler.h>
#endif // __U_PROFILER__

#include <uDebug.h>										// access: uDebugWrite
#undef __U_DEBUG_H__									// turn off debug prints

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <climits>										// ULONG_MAX
#include <new>
#include <unistd.h>										// sbrk, sysconf

#define LIKELY(x)       __builtin_expect(!!(x), 1)
#define UNLIKELY(x)     __builtin_expect(!!(x), 0)

#define Fence() __asm__ __volatile__ ( "lock; addq $0,(%%rsp);" ::: "cc" )


namespace UPP {
	#ifdef __U_DEBUG__
	static bool uHeapBoot = false;						// detect recursion during boot
	#endif // __U_DEBUG__
	// The constructor for heapManager is called explicitly in uHeapManager::boot using placement syntax.
	static char uHeapStorage[sizeof(uHeapManager)] __attribute__(( aligned (128) )) = {0}; // size of cache line to prevent false sharing

	uHeapManager * uHeapManager::heapManagerInstance = nullptr;
	size_t uHeapManager::pageSize;						// architecture pagesize
	size_t uHeapManager::heapExpand;					// sbrk advance
	size_t uHeapManager::mmapStart;						// cross over point for mmap
	unsigned int uHeapManager::maxBucketsUsed;			// maximum number of buckets in use

	// Bucket size must be multiple of 16.
	// Powers of 2 are common allocation sizes, so make powers of 2 generate the minimum required size.
	const unsigned int uHeapManager::bucketSizes[] = {	// different bucket sizes
		16 + sizeof(uHeapManager::Storage), 32 + sizeof(uHeapManager::Storage), 48 + sizeof(uHeapManager::Storage), 64 + sizeof(uHeapManager::Storage), // 4
		96 + sizeof(uHeapManager::Storage), 112 + sizeof(uHeapManager::Storage), 128 + sizeof(uHeapManager::Storage), // 3
		160, 192, 224, 256 + sizeof(uHeapManager::Storage), // 4
		320, 384, 448, 512 + sizeof(uHeapManager::Storage), // 4
		640, 768, 896, 1024 + sizeof(uHeapManager::Storage), // 4
		1536, 2048 + sizeof(uHeapManager::Storage), // 2
		2560, 3072, 3584, 4096 + sizeof(uHeapManager::Storage), // 4
		6144, 8192 + sizeof(uHeapManager::Storage), // 2
		9216, 10240, 11264, 12288, 13312, 14336, 15360, 16384 + sizeof(uHeapManager::Storage), // 8
		18432, 20480, 22528, 24576, 26624, 28672, 30720, 32768 + sizeof(uHeapManager::Storage), // 8
		36864, 40960, 45056, 49152, 53248, 57344, 61440, 65536 + sizeof(uHeapManager::Storage), // 8
		73728, 81920, 90112, 98304, 106496, 114688, 122880, 131072 + sizeof(uHeapManager::Storage), // 8
		147456, 163840, 180224, 196608, 212992, 229376, 245760, 262144 + sizeof(uHeapManager::Storage), // 8
		294912, 327680, 360448, 393216, 425984, 458752, 491520, 524288 + sizeof(uHeapManager::Storage), // 8
		655360, 786432, 917504, 1048576 + sizeof(uHeapManager::Storage), // 4
		1179648, 1310720, 1441792, 1572864, 1703936, 1835008, 1966080, 2097152 + sizeof(uHeapManager::Storage), // 8
		2621440, 3145728, 3670016, 4194304 + sizeof(uHeapManager::Storage) // 4
	};
	// FIX ME
	//static_assert( uHeapManager::NoBucketSizes == sizeof(uHeapManager::bucketSizes) / sizeof(uHeapManager::bucketSizes[0]), "size of bucket array wrong" );
	#ifdef FASTLOOKUP
	unsigned char uHeapManager::lookup[];				// array size defined in .h
	#endif // FASTLOOKUP

	int uHeapManager::mmapFd = -1;						// fake or actual fd for anonymous file
	#ifdef __U_DEBUG__
	unsigned long int uHeapManager::allocUnfreed = 0;
	#endif // __U_DEBUG__

	#ifdef __U_STATISTICS__
	// Heap statistics counters.
	unsigned int uHeapManager::malloc_calls = 0;
	unsigned long long int uHeapManager::malloc_storage = 0;
	unsigned int uHeapManager::aalloc_calls = 0;
	unsigned long long int uHeapManager::aalloc_storage = 0;
	unsigned int uHeapManager::calloc_calls = 0;
	unsigned long long int uHeapManager::calloc_storage = 0;
	unsigned int uHeapManager::memalign_calls = 0;
	unsigned long long int uHeapManager::memalign_storage = 0;
	unsigned int uHeapManager::amemalign_calls = 0;
	unsigned long long int uHeapManager::amemalign_storage = 0;
	unsigned int uHeapManager::cmemalign_calls = 0;
	unsigned long long int uHeapManager::cmemalign_storage = 0;
	unsigned int uHeapManager::resize_calls = 0;
	unsigned long long int uHeapManager::resize_storage = 0;
	unsigned int uHeapManager::realloc_calls = 0;
	unsigned long long int uHeapManager::realloc_storage = 0;
	unsigned int uHeapManager::free_calls = 0;
	unsigned long long int uHeapManager::free_storage = 0;
	unsigned int uHeapManager::mmap_calls = 0;
	unsigned long long int uHeapManager::mmap_storage = 0;
	unsigned int uHeapManager::munmap_calls = 0;
	unsigned long long int uHeapManager::munmap_storage = 0;
	unsigned int uHeapManager::sbrk_calls = 0;
	unsigned long long int uHeapManager::sbrk_storage = 0;
	// Statistics file descriptor (changed by malloc_stats_fd).
	int uHeapManager::stats_fd = STDERR_FILENO;		// default stderr

	// Use "write" because streams may be shutdown when calls are made.
	void uHeapManager::printStats() {
		char helpText[1024];
		int len = snprintf( helpText, sizeof(helpText),
							"\nHeap statistics:\n"
							"  malloc: calls %u / storage %llu\n"
							"  aalloc: calls %u / storage %llu\n"
							"  calloc: calls %u / storage %llu\n"
							"  memalign: calls %u / storage %llu\n"
							"  amemalign: calls %u / storage %llu\n"
							"  cmemalign: calls %u / storage %llu\n"
							"  resize: calls %u / storage %llu\n"
							"  realloc: calls %u / storage %llu\n"
							"  free: calls %u / storage %llu\n"
							"  mmap: calls %u / storage %llu\n"
							"  munmap: calls %u / storage %llu\n"
							"  sbrk: calls %u / storage %llu\n",
							malloc_calls, malloc_storage,
							aalloc_calls, aalloc_storage,
							calloc_calls, calloc_storage,
							memalign_calls, memalign_storage,
							amemalign_calls, amemalign_storage,
							cmemalign_calls, cmemalign_storage,
							resize_calls, resize_storage,
							realloc_calls, realloc_storage,
							free_calls, free_storage,
							mmap_calls, mmap_storage,
							munmap_calls, munmap_storage,
							sbrk_calls, sbrk_storage
			);
		uDebugWrite( stats_fd, helpText, len );
	} // uHeapManager::printStats

	int uHeapManager::printStatsXML( FILE * stream ) {
		char helpText[1024];
		int len = snprintf( helpText, sizeof(helpText),
							"<malloc version=\"1\">\n"
							"<heap nr=\"0\">\n"
							"<sizes>\n"
							"</sizes>\n"
							"<total type=\"malloc\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"aalloc\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"calloc\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"memalign\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"amemalign\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"cmemalign\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"resize\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"realloc\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"free\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"mmap\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"munmap\" count=\"%u\" size=\"%llu\"/>\n"
							"<total type=\"sbrk\" count=\"%u\" size=\"%llu\"/>\n"
							"</malloc>",
							malloc_calls, malloc_storage,
							aalloc_calls, aalloc_storage,
							calloc_calls, calloc_storage,
							memalign_calls, memalign_storage,
							amemalign_calls, amemalign_storage,
							cmemalign_calls, cmemalign_storage,
							resize_calls, resize_storage,
							realloc_calls, realloc_storage,
							free_calls, free_storage,
							mmap_calls, mmap_storage,
							munmap_calls, munmap_storage,
							sbrk_calls, sbrk_storage
			);
		uDebugWrite( fileno( stream ), helpText, len );	// ensures all bytes written or exit
		return len;
	} // printStatsXML
	#endif // __U_STATISTICS__


	inline void uHeapManager::noMemory() {
		abort( "Heap memory exhausted at %zu bytes.\n"
			   "Possible cause is very large memory allocation and/or large amount of unfreed storage allocated by the program or system/library routines.",
			   ((char *)(sbrk( 0 )) - (char *)(uHeapManager::heapManagerInstance->heapBegin)) );
	} // uHeapManager::noMemory


	bool uHeapManager::setMmapStart( size_t value ) {	// true => mmapped, false => sbrk
	  if ( value < pageSize || bucketSizes[NoBucketSizes - 1] < value ) return false;
		mmapStart = value;								// set global

		// find the closest bucket size less than or equal to the mmapStart size
		maxBucketsUsed = std::lower_bound( bucketSizes, bucketSizes + (NoBucketSizes - 1), mmapStart ) - bucketSizes; // binary search
		assert( maxBucketsUsed < NoBucketSizes );		// subscript failure ?
		assert( mmapStart <= bucketSizes[maxBucketsUsed] ); // search failure ?
		return true;
	} // uHeapManager::setMmapStart


	// <-------+----------------------------------------------------> bsize (bucket size)
	// |header |addr
	//==================================================================================
	//                   align/offset |
	// <-----------------<------------+-----------------------------> bsize (bucket size)
	//                   |fake-header | addr
	#define headerAddr( addr ) ((UPP::uHeapManager::Storage::Header *)( (char *)addr - sizeof(UPP::uHeapManager::Storage) ))
	#define realHeader( header ) ((UPP::uHeapManager::Storage::Header *)((char *)header - header->kind.fake.offset))

	// <-------<<--------------------- dsize ---------------------->> bsize (bucket size)
	// |header |addr
	//==================================================================================
	//                   align/offset |
	// <------------------------------<<---------- dsize --------->>> bsize (bucket size)
	//                   |fake-header |addr
	#define dataStorage( bsize, addr, header ) (bsize - ( (char *)addr - (char *)header ))


	inline void uHeapManager::checkAlign( size_t alignment ) {
		if ( alignment < uAlign() || ! uPow2( alignment ) ) {
			abort( "Alignment %zu for memory allocation is less than %d and/or not a power of 2.", alignment, uAlign() );
		} // if
	} // uHeapManager::checkAlign

	static inline void checkHeader( bool check, const char name[], void * addr ) {
		if ( UNLIKELY( check ) ) {						// bad address ?
			abort( "Attempt to %s storage %p with address outside the heap.\n"
				   "Possible cause is duplicate free on same block or overwriting of memory.",
				   name, addr );
		} // if
	} // checkHeader

	inline void uHeapManager::fakeHeader( Storage::Header *& header, size_t & alignment ) {
		if ( UNLIKELY( (header->kind.fake.alignment & 1) == 1 ) ) { // fake header ?
			alignment = header->kind.fake.alignment & -2; // remove flag from value
			#ifdef __U_DEBUG__
			checkAlign( alignment );					// check alignment
			#endif // __U_DEBUG__
			header = realHeader( header );				// backup from fake to real header
		} else {
			alignment = uAlign();						// => no fake header
		} // if
	} // uHeapManager::fakeHeader

	inline bool uHeapManager::headers( const char name[] __attribute__(( unused )), void * addr, Storage::Header *& header, FreeHeader *& freeElem, size_t & size, size_t & alignment ) {
		header = headerAddr( addr );

	  if ( UNLIKELY( addr < heapBegin || heapEnd < addr ) ) { // mmapped ?
			fakeHeader( header, alignment );
			size = header->kind.real.blockSize & -3;	// mmap size
			return true;
		} // if

		#ifdef __U_DEBUG__
		checkHeader( header < heapBegin, name, addr );	// bad low address ?
		#endif // __U_DEBUG__

		// header may be safe to dereference
		fakeHeader( header, alignment );
		#ifdef __U_DEBUG__
		checkHeader( header < heapBegin || heapEnd < header, name, addr ); // bad address ? (offset could be + or -)
		#endif // __U_DEBUG__

		freeElem = (FreeHeader *)((size_t)header->kind.real.home & -3);
		#ifdef __U_DEBUG__
		if ( freeElem < &freeLists[0] || &freeLists[NoBucketSizes] <= freeElem ) {
			abort( "Attempt to %s storage %p with corrupted header.\n"
				   "Possible cause is duplicate free on same block or overwriting of header information.",
				   name, addr );
		} // if
		#endif // __U_DEBUG__
		size = freeElem->blockSize;
		return false;
	} // uHeapManager::headers


	// #ifdef __U_DEBUG__
	// #if __SIZEOF_POINTER__ == 4
	// #define MASK 0xdeadbeef
	// #else
	// #define MASK 0xdeadbeefdeadbeef
	// #endif
	// #define STRIDE size_t

	// static void * Memset( void * addr, STRIDE size ) { // debug only
	// 	if ( size % sizeof(STRIDE) != 0 ) abort( "Memset() : internal error, size %zd not multiple of %zd.", size, sizeof(STRIDE) );
	// 	if ( (STRIDE)addr % sizeof(STRIDE) != 0 ) abort( "Memset() : internal error, addr %p not multiple of %zd.", addr, sizeof(STRIDE) );

	// 	STRIDE * end = (STRIDE *)addr + size / sizeof(STRIDE);
	// 	for ( STRIDE * p = (STRIDE *)addr; p < end; p += 1 ) *p = MASK;
	// 	return addr;
	// } // Memset
	// #endif // __U_DEBUG__

	
	#define NO_MEMORY_MSG "insufficient heap memory available for allocating %zd new bytes."

	inline void * uHeapManager::extend( size_t size ) {
		extlock.acquire();
		uDEBUGPRT( uDebugPrt( "(uHeapManager &)%p.extend( %zu ), heapBegin:%p, heapEnd:%p, heapRemaining:0x%zx, sbrk:%p\n",
							  this, size, heapBegin, heapEnd, heapRemaining, sbrk(0) ); )
		ptrdiff_t rem = heapRemaining - size;
		if ( rem < 0 ) {
			// If the size requested is bigger than the current remaining storage, increase the size of the heap.

			size_t increase = uCeiling( size > heapExpand ? size : heapExpand, uAlign() );
			if ( sbrk( increase ) == (void *)-1 ) {		// failed, no memory ?
				uDEBUGPRT( uDebugPrt( "0x%zx = (uHeapManager &)%p.extend( %zu ), heapBegin:%p, heapEnd:%p, heapRemaining:0x%zx, sbrk:%p\n",
									  nullptr, this, size, heapBegin, heapEnd, heapRemaining, sbrk(0) ); )
				extlock.release();
				uDebugPrt( NO_MEMORY_MSG, size );		// give up
			} // if
			#ifdef __U_STATISTICS__
			sbrk_calls += 1;
			sbrk_storage += increase;
			#endif // __U_STATISTICS__
			#ifdef __U_DEBUG__
			// Set new memory to garbage so subsequent uninitialized usages might fail.
			memset( (char *)heapEnd + heapRemaining, '\xde', increase );
			//Memset( (char *)heapEnd + heapRemaining, increase );
			#endif // __U_DEBUG__
			rem = heapRemaining + increase - size;
		} // if

		Storage * block = (Storage *)heapEnd;
		heapRemaining = rem;
		heapEnd = (char *)heapEnd + size;
		uDEBUGPRT( uDebugPrt( "%p = (uHeapManager &)%p.extend( %zu ), heapBegin:%p, heapEnd:%p, heapRemaining:0x%zx, sbrk:%p\n",
							  block, this, size, heapBegin, heapEnd, heapRemaining, sbrk(0) ); )
		extlock.release();
		return block;
	} // uHeapManager::extend


	inline void * uHeapManager::doMalloc( size_t size ) {
		uDEBUGPRT( uDebugPrt( "(uHeapManager &)%p.doMalloc( %zu )\n", this, size ); )

		Storage * block;

		// Look up size in the size list.  Make sure the user request includes space for the header that must be allocated
		// along with the block and is a multiple of the alignment size.

	  if ( UNLIKELY( size > ULONG_MAX - sizeof(Storage) ) ) return nullptr;
		size_t tsize = size + sizeof(Storage);
		if ( LIKELY( tsize < mmapStart ) ) {			// small size => sbrk
			FreeHeader * freeElem =
				#ifdef FASTLOOKUP
				tsize < LookupSizes ? &freeLists[lookup[tsize]] :
				#endif // FASTLOOKUP
				std::lower_bound( freeLists, freeLists + maxBucketsUsed, tsize ); // binary search
			assert( freeElem <= &freeLists[maxBucketsUsed] ); // subscripting error ?
			assert( tsize <= freeElem->blockSize );		// search failure ?
			tsize = freeElem->blockSize;				// total space needed for request

			uDEBUGPRT( uDebugPrt( "(uHeapManager &)%p.doMalloc, size after lookup:%zu\n", this, tsize ); )
	
			// Spin until the lock is acquired for this particular size of block.

			#if BUCKETLOCK == SPINLOCK
			freeElem->lock.acquire();
			block = freeElem->freeList;					// remove node from stack
			#else
			block = freeElem->freeList.pop();
			#endif // BUCKETLOCK
			if ( UNLIKELY( block == nullptr ) ) {		// no free block ?
				#if BUCKETLOCK == SPINLOCK
				freeElem->lock.release();
				#endif // BUCKETLOCK
				// Freelist for that size was empty, so carve it out of the heap if there's enough left, or get some more
				// and then carve it off.

				block = (Storage *)extend( tsize );		// mutual exclusion on call
			#if BUCKETLOCK == SPINLOCK
			} else {
				freeElem->freeList = block->header.kind.real.next;
				freeElem->lock.release();
			#endif // BUCKETLOCK
			} // if

			block->header.kind.real.home = freeElem;	// pointer back to free list of apropriate size
		} else {										// large size => mmap
	  if ( UNLIKELY( size > ULONG_MAX - pageSize ) ) return nullptr;
			tsize = uCeiling( tsize, pageSize );		// must be multiple of page size
			#ifdef __U_STATISTICS__
			uFetchAdd( mmap_calls, 1 );
			uFetchAdd( mmap_storage, tsize );
			#endif // __U_STATISTICS__

			block = (Storage *)::mmap( 0, tsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, mmapFd, 0 );
			if ( block == MAP_FAILED ) { // failed ?
				if ( errno == ENOMEM ) abort( NO_MEMORY_MSG, tsize ); // no memory
				// Do not call strerror( errno ) as it may call malloc.
				abort( "(uHeapManager &)0x%p.doMalloc() : internal error, mmap failure, size:%zu error:%d.", this, tsize, errno );
			} // if
			#ifdef __U_DEBUG__
			// Set new memory to garbage so subsequent uninitialized usages might fail.
			memset( block, '\xde', tsize );
			//Memset( block, tsize );
			#endif // __U_DEBUG__
			block->header.kind.real.blockSize = tsize;	// storage size for munmap
		} // if

		block->header.kind.real.size = size;			// store allocation size
		void * addr = &(block->data);					// adjust off header to user bytes
		assert( ((uintptr_t)addr & (uAlign() - 1)) == 0 ); // minimum alignment ?

		#ifdef __U_DEBUG__
		uFetchAdd( uHeapManager::allocUnfreed, tsize );
		if ( uHeapControl::traceHeap() ) {
			enum { BufferSize = 64 };
			char helpText[BufferSize];
			int len = snprintf( helpText, BufferSize, "%p = Malloc( %zu ) (allocated %zu)\n", addr, size, tsize );
			uDebugWrite( STDERR_FILENO, helpText, len );
		} // if
		#endif // __U_DEBUG__

		uDEBUGPRT( uDebugPrt( "%p = (uHeapManager &)%p.doMalloc\n", addr, this ); )
		return addr;
	} // uHeapManager::doMalloc

	inline void uHeapManager::doFree( void * addr ) {
		uDEBUGPRT( uDebugPrt( "(uHeapManager &)%p.doFree( %p )\n", this, addr ); )

		#ifdef __U_DEBUG__
		if ( UNLIKELY( uHeapManager::heapManagerInstance == nullptr ) ) {
			abort( "uHeapManager::doFree( %p ) : internal error, called before heap is initialized.", addr );
		} // if
		#endif // __U_DEBUG__

		Storage::Header * header;
		FreeHeader * freeElem;
		size_t size, alignment;							// not used (see realloc)

		if ( headers( "free", addr, header, freeElem, size, alignment ) ) { // mmapped ?
			#ifdef __U_STATISTICS__
			uFetchAdd( munmap_calls, 1 );
			uFetchAdd( munmap_storage, size );
			#endif // __U_STATISTICS__
			if ( munmap( header, size ) == -1 ) {
				abort( "Attempt to deallocate storage %p not allocated or with corrupt header.\n"
					   "Possible cause is invalid pointer.",
					   addr );
			} // if
		} else {
			#ifdef __U_PROFILER__
			if ( uThisTask().profileActive && uProfiler::uProfiler_registerMemoryDeallocate ) {
				(* uProfiler::uProfiler_registerMemoryDeallocate)( uProfiler::profilerInstance, addr, freeElem->blockSize, PROFILEMALLOCENTRY( header ) ); 
			} // if
			#endif // __U_PROFILER__

			#ifdef __U_DEBUG__
			// Set free memory to garbage so subsequent usages might fail.
			memset( ((Storage *)header)->data, '\xde', freeElem->blockSize - sizeof( Storage ) );
			//Memset( ((Storage *)header)->data, freeElem->blockSize - sizeof( Storage ) );
			#endif // __U_DEBUG__

			uDEBUGPRT( uDebugPrt( "(uHeapManager &)%p.doFree( %p ) header:%p freeElem:%p\n", this, addr, &header, &freeElem ); )

			#ifdef __U_STATISTICS__
			free_storage += size;
			#endif // __U_STATISTICS__
			#if BUCKETLOCK == SPINLOCK
			freeElem->lock.acquire();					// acquire spin lock
			header->kind.real.next = freeElem->freeList; // push on stack
			freeElem->freeList = (Storage *)header;
			freeElem->lock.release();					// release spin lock
			#else
			freeElem->freeList.push( *(Storage *)header );
			#endif // BUCKETLOCK
			uDEBUGPRT( uDebugPrt( "(uHeapManager &)%p.doFree( %p ) returning free block in list 0x%zx\n", this, addr, size ); )
		} // if

		#ifdef __U_DEBUG__
		uFetchAdd( uHeapManager::allocUnfreed, -size );
		if ( uHeapControl::traceHeap() ) {
			char helpText[64];
			int len = snprintf( helpText, sizeof(helpText), "Free( %p ) size:%zu\n", addr, size );
			uDebugWrite( STDERR_FILENO, helpText, len ); // print debug/nodebug
		} // if
		#endif // __U_DEBUG__
	} // uHeapManager::doFree


	size_t uHeapManager::prtFree() {
		size_t total = 0;
		#ifdef __U_STATISTICS__
		uDebugAcquire();
		uDebugPrt2( "\nBin lists (bin size : free blocks on list)\n" );
		#endif // __U_STATISTICS__
		for ( unsigned int i = 0; i < maxBucketsUsed; i += 1 ) {
			size_t size = freeLists[i].blockSize;
			#ifdef __U_STATISTICS__
			unsigned int N = 0;
			#endif // __U_STATISTICS__

			#if BUCKETLOCK == SPINLOCK
			for ( Storage * p = freeLists[i].freeList; p != nullptr; p = p->header.kind.real.next ) {
			#else
			for ( Storage * p = freeLists[i].freeList.top(); p != nullptr; p = p->getNext()->top ) {
			#endif // BUCKETLOCK
				total += size;
				#ifdef __U_STATISTICS__
				N += 1;
				#endif // __U_STATISTICS__
			} // for
			#ifdef __U_STATISTICS__
			uDebugPrt2( "%7zu, %-7u  ", size, N );
			if ( (i + 1) % 8 == 0 ) uDebugPrt2( "\n" );
			#endif // __U_STATISTICS__
		} // for
		#ifdef __U_STATISTICS__
		uDebugPrt2( "\ntotal free blocks:%zu\n", total );
		uDebugRelease();
		#endif // __U_STATISTICS__
		return (char *)heapEnd - (char *)heapBegin - total;
	} // uHeapManager::prtFree


	uHeapManager::uHeapManager() {
		uDEBUGPRT( uDebugPrt( "(uHeapManager &)%p.uHeap()\n", this ); )
		pageSize = sysconf( _SC_PAGESIZE );

		for ( unsigned int i = 0; i < NoBucketSizes; i += 1 ) { // initialize the free lists
			freeLists[i].blockSize = bucketSizes[i];
		} // for

		#ifdef FASTLOOKUP
		unsigned int idx = 0;
		for ( unsigned int i = 0; i < LookupSizes; i += 1 ) {
			if ( i > bucketSizes[idx] ) idx += 1;
			lookup[i] = idx;
		} // for
		#endif // FASTLOOKUP

		if ( ! setMmapStart( uDefaultMmapStart() ) ) {
			abort( "uHeapManager::uHeapManager : internal error, mmap start initialization failure." );
		} // if
		heapExpand = uDefaultHeapExpansion();

		char * end = (char *)sbrk( 0 );
		heapBegin = heapEnd = sbrk( (char *)uCeiling( (long unsigned int)end, uAlign() ) - end ); // move start of heap to multiple of alignment

		uDEBUGPRT( uDebugPrt( "(uHeapManager &)%p.uHeap() heapBegin:%p, heapEnd:%p\n", this, heapBegin, heapEnd ); )
	} // uHeapManager::uHeapManager


	uHeapManager::~uHeapManager() {
		#ifdef __U_STATISTICS__
		if ( UPP::uHeapControl::prtHeapTerm() ) {
			printStats();
			if ( UPP::uHeapControl::prtFree() ) uHeapManager::heapManagerInstance->prtFree();
		} // if
		#endif // __U_STATISTICS__

		#ifdef __U_DEBUG__
		if ( allocUnfreed != 0 ) {
			// DO NOT USE STREAMS AS THEY MAY BE UNAVAILABLE AT THIS POINT.
			char helpText[512];
			int len = snprintf( helpText, sizeof(helpText), "uC++ Runtime warning (UNIX pid:%ld) : program terminating with %zu(0x%zx) bytes of storage allocated but not freed.\n"
								"Possible cause is unfreed storage allocated by the program or system/library routines called from the program.\n",
								(long int)getpid(), allocUnfreed, allocUnfreed ); // always print the UNIX pid
			uDebugWrite( STDERR_FILENO, helpText, len );
		} // if
		#endif // __U_DEBUG__
	} // uHeapManager::~uHeapManager


	void uHeapManager::boot() {
		uDEBUGPRT( uDebugPrt( "uHeapManager::boot() enter\n" ); )
		if ( ! uKernelModule::kernelModuleInitialized ) {
			uKernelModule::startup();
		} // if

		#ifdef __U_DEBUG__
		if ( uHeapBoot ) {								// check for recursion during system boot
			abort( "uHeapManager::boot() : internal error, recursively invoked during system boot." );
		} // if
		uHeapBoot = true;
		#endif // __U_DEBUG__

		uHeapManager::heapManagerInstance = new( &uHeapStorage ) uHeapManager;

		std::set_new_handler( noMemory );				// do not throw exception as the default

		uDEBUGPRT( uDebugPrt( "uHeapManager::boot() exit\n" ); )
	} // uHeapManager::boot


	void * uHeapManager::operator new( size_t, void * storage ) {
		return storage;
	} // uHeapManager::operator new


	void * uHeapManager::operator new( size_t size ) {
		return ::operator new( size );
	} // uHeapManager::operator new


	void uHeapControl::startup() {
		// Just in case no previous malloc, initialization of heap.

		if ( uHeapManager::heapManagerInstance == nullptr ) {
			uHeapManager::boot();
		} // if

		// Storage allocated before the start of uC++ is normally not freed until after uC++ completes (if at all). Hence,
		// this storage is not considered when calculating unfreed storage when the heap's destructor is called in finishup.

		#ifdef __U_DEBUG__
		uHeapManager::allocUnfreed = 0;
		#endif // __U_DEBUG__
	} // uHeapControl::startup

	void uHeapControl::finishup() {
		// Explicitly invoking the destructor does not close down the heap because it might still be used before the
		// application terminates. The heap's destructor does check for unreleased storage at this point. (The constructor
		// for the heap is called on the first call to malloc.)

		uHeapManager::heapManagerInstance->uHeapManager::~uHeapManager();
	} // uHeapControl::finishup

	void uHeapControl::prepareTask( uBaseTask * /* task */ ) {
	} // uHeapControl::prepareTask

	// void uHeapControl::startTask() {
	// } // uHeapControl::startTask

	// void uHeapControl::finishTask() {
	// } // uHeapControl::finishTask


	inline void * uHeapManager::mallocNoStats( size_t size ) __THROW { // necessary for malloc statistics
		if ( UNLIKELY( UPP::uHeapManager::heapManagerInstance == nullptr ) ) {
			UPP::uHeapManager::boot();
		} // if
	  if ( UNLIKELY( size ) == 0 ) return nullptr;		// 0 BYTE ALLOCATION RETURNS NULL POINTER

		void * addr = heapManagerInstance->doMalloc( size );

		#ifdef __U_PROFILER__
		if ( uThisTask().profileActive && uProfiler::uProfiler_registerMemoryAllocate ) {
			Storage::Header * header = headerAddr( addr );
			PROFILEMALLOCENTRY( header ) = (* uProfiler::uProfiler_registerMemoryAllocate)( uProfiler::profilerInstance, addr, size, header->kind.real.blockSize & -3 );
		} // if
		#endif // __U_PROFILER__

		return addr;
	} // mallocNoStats


	inline void * uHeapManager::callocNoStats( size_t dim, size_t elemSize ) __THROW {
		size_t size = dim * elemSize;
	  if ( UNLIKELY( size ) == 0 ) return nullptr;		// 0 BYTE ALLOCATION RETURNS NULL POINTER
		char * addr = (char *)mallocNoStats( size );

		Storage::Header * header;
		FreeHeader * freeElem;
		size_t bsize, alignment;
		#ifndef __U_DEBUG__
		bool mapped =
		#endif // __U_DEBUG__
			heapManagerInstance->headers( "calloc", addr, header, freeElem, bsize, alignment );
		#ifndef __U_DEBUG__

		// Mapped storage is zero filled, but in debug mode mapped memory is scrubbed in doMalloc, so it has to be reset to zero.
		if ( ! mapped )
		#endif // __U_DEBUG__
			// <-------0000000000000000000000000000UUUUUUUUUUUUUUUUUUUUUUUUU> bsize (bucket size) U => undefined
			// `-header`-addr                      `-size
			memset( addr, '\0', size );					// set to zeros

		header->kind.real.blockSize |= 2;				// mark as zero filled
		return addr;
	} // callocNoStats


	inline void * uHeapManager::memalignNoStats( size_t alignment, size_t size ) __THROW {
	  if ( UNLIKELY( size ) == 0 ) return nullptr;		// 0 BYTE ALLOCATION RETURNS NULL POINTER

		#ifdef __U_DEBUG__
		checkAlign( alignment );						// check alignment
		#endif // __U_DEBUG__

		// if alignment <= default alignment, do normal malloc as two headers are unnecessary
	  if ( UNLIKELY( alignment <= uAlign() ) ) return mallocNoStats( size );

		// Allocate enough storage to guarantee an address on the alignment boundary, and sufficient space before it for
		// administrative storage. NOTE, WHILE THERE ARE 2 HEADERS, THE FIRST ONE IS IMPLICITLY CREATED BY DOMALLOC.
		//      .-------------v-----------------v----------------v----------,
		//      | Real Header | ... padding ... |   Fake Header  | data ... |
		//      `-------------^-----------------^-+--------------^----------'
		//      |<--------------------------------' offset/align |<-- alignment boundary

		// subtract uAlign() because it is already the minimum alignment
		// add sizeof(Storage) for fake header
		char * addr = (char *)mallocNoStats( size + alignment - uAlign() + sizeof(Storage) );

		// address in the block of the "next" alignment address
		char * user = (char *)uCeiling( (uintptr_t)(addr + sizeof(Storage)), alignment );

		// address of header from malloc
		Storage::Header * realHeader = headerAddr( addr );
		realHeader->kind.real.size = size;				// correct size to eliminate above alignment offset
		// address of fake header * before* the alignment location
		Storage::Header * fakeHeader = headerAddr( user );
		// SKULLDUGGERY: insert the offset to the start of the actual storage block and remember alignment
		fakeHeader->kind.fake.offset = (char *)fakeHeader - (char *)realHeader;
		// SKULLDUGGERY: odd alignment imples fake header
		fakeHeader->kind.fake.alignment = alignment | 1;

		#ifdef __U_PROFILER__
		if ( uThisTask().profileActive && uProfiler::uProfiler_registerMemoryAllocate ) {
			PROFILEMALLOCENTRY( fakeHeader ) = (* uProfiler::uProfiler_registerMemoryAllocate)( uProfiler::profilerInstance, addr, size, realHeader->kind.real.home->blockSize & -3 );
		} // if
		#endif // __U_PROFILER__

		return user;
	} // memalignNoStats


	inline void * uHeapManager::cmemalignNoStats( size_t alignment, size_t dim, size_t elemSize ) __THROW {
		size_t size = dim * elemSize;
	  if ( UNLIKELY( size ) == 0 ) return nullptr;		// 0 BYTE ALLOCATION RETURNS NULL POINTER
		char * addr = (char *)memalignNoStats( alignment, size );

		Storage::Header * header;
		FreeHeader * freeElem;
		size_t bsize;
		#ifndef __U_DEBUG__
		bool mapped =
		#endif // __U_DEBUG__
			heapManagerInstance->headers( "cmemalign", addr, header, freeElem, bsize, alignment );

		// Mapped storage is zero filled, but in debug mode mapped memory is scrubbed in doMalloc, so it has to be reset to zero.
		#ifndef __U_DEBUG__
		if ( ! mapped )
		#endif // __U_DEBUG__
			// <-------0000000000000000000000000000UUUUUUUUUUUUUUUUUUUUUUUUU> bsize (bucket size) U => undefined
			// `-header`-addr                      `-size
			memset( addr, '\0', size );					// set to zeros

		header->kind.real.blockSize |= 2;				// mark as zero filled
		return addr;
	} // cmemalignNoStats
} // UPP


// Operators new and new [] call malloc; delete calls free

extern "C" {
	// Allocates size bytes and returns a pointer to the allocated memory.  The contents are undefined. If size is 0,
	// then malloc() returns a unique pointer value that can later be successfully passed to free().
	void * malloc( size_t size ) __THROW {
		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::malloc_calls, 1 );
		uFetchAdd( UPP::uHeapManager::malloc_storage, size );
		#endif // __U_STATISTICS__

		return UPP::uHeapManager::mallocNoStats( size );
	} // malloc


	// Same as malloc() except size bytes is an array of dim elements each of elemSize bytes.
	void * aalloc( size_t dim, size_t elemSize ) __THROW {
		size_t size = dim * elemSize;
		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::aalloc_calls, 1 );
		uFetchAdd( UPP::uHeapManager::aalloc_storage, size );
		#endif // __U_STATISTICS__

		return UPP::uHeapManager::mallocNoStats( size );
	} // aalloc


	// Same as aalloc() with memory set to zero.
	void * calloc( size_t dim, size_t elemSize ) __THROW {
		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::calloc_calls, 1 );
		uFetchAdd( UPP::uHeapManager::calloc_storage, dim * elemSize );
		#endif // __U_STATISTICS__

		return UPP::uHeapManager::callocNoStats( dim, elemSize );
	} // calloc


	// Change the size of the memory block pointed to by oaddr to size bytes. The contents are undefined.  If oaddr is
	// nullptr, then the call is equivalent to malloc(size), for all values of size; if size is equal to zero, and oaddr is
	// not nullptr, then the call is equivalent to free(oaddr). Unless oaddr is nullptr, it must have been returned by an earlier
	// call to malloc(), alloc(), calloc() or realloc(). If the area pointed to was moved, a free(oaddr) is done.
	void * resize( void * oaddr, size_t size ) __THROW {
		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::resize_calls, 1 );
		#endif // __U_STATISTICS__

		// If size is equal to 0, either NULL or a pointer suitable to be passed to free() is returned.
	  if ( UNLIKELY( size == 0 ) ) { free( oaddr ); return nullptr; } // special cases
	  if ( UNLIKELY( oaddr == nullptr ) ) {
			#ifdef __U_STATISTICS__
			uFetchAdd( UPP::uHeapManager::resize_storage, size );
			#endif // __U_STATISTICS__
			return UPP::uHeapManager::mallocNoStats( size );
		} // if

		UPP::uHeapManager::Storage::Header * header;
		UPP::uHeapManager::FreeHeader * freeElem;
		size_t bsize, oalign;
		UPP::uHeapManager::heapManagerInstance->headers( "resize", oaddr, header, freeElem, bsize, oalign );
		size_t odsize = dataStorage( bsize, oaddr, header ); // data storage available in bucket

		// same size, DO NOT preserve STICKY PROPERTIES.
		if ( oalign == uAlign() && size <= odsize && odsize <= size * 2 ) { // allow 50% wasted storage for smaller size
			header->kind.real.blockSize &= -2; // no alignment and turn off 0 fill
			header->kind.real.size = size;	// reset allocation size
			return oaddr;
		} // if

		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::resize_storage, size );
		#endif // __U_STATISTICS__

		// change size, DO NOT preserve STICKY PROPERTIES.
		free( oaddr );
		return UPP::uHeapManager::mallocNoStats( size ); // create new area
	} // resize


	// Same as resize() but the contents are unchanged in the range from the start of the region up to the minimum of
	// the old and new sizes.
	void * realloc( void * oaddr, size_t size ) __THROW {
		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::realloc_calls, 1 );
		#endif // __U_STATISTICS__

		// If size is equal to 0, either NULL or a pointer suitable to be passed to free() is returned.
	  if ( UNLIKELY( size == 0 ) ) { free( oaddr ); return nullptr; } // special cases
	  if ( UNLIKELY( oaddr == nullptr ) ) {
			#ifdef __U_STATISTICS__
			uFetchAdd( UPP::uHeapManager::realloc_storage, size );
			#endif // __U_STATISTICS__
			return UPP::uHeapManager::mallocNoStats( size );
		} // if

		UPP::uHeapManager::Storage::Header * header;
		UPP::uHeapManager::FreeHeader * freeElem;
		size_t bsize, oalign;
		UPP::uHeapManager::heapManagerInstance->headers( "realloc", oaddr, header, freeElem, bsize, oalign );

		size_t odsize = dataStorage( bsize, oaddr, header ); // data storage available in bucket
		size_t osize = header->kind.real.size;			// old allocation size
		bool ozfill = (header->kind.real.blockSize & 2); // old allocation zero filled
	  if ( UNLIKELY( size <= odsize ) && odsize <= size * 2 ) { // allow up to 50% wasted storage
	  		header->kind.real.size = size;				// reset allocation size
	  		if ( UNLIKELY( ozfill ) && size > osize ) {	// previous request zero fill and larger ?
	  			memset( (char *)oaddr + osize, '\0', size - osize ); // initialize added storage
	  		} // if
			return oaddr;
		} // if

		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::realloc_storage, size );
		#endif // __U_STATISTICS__

	  // change size and copy old content to new storage

	  void * naddr;
	  if ( UNLIKELY( oalign <= uAlign() ) ) {			// previous request not aligned ?
		  naddr = UPP::uHeapManager::mallocNoStats( size );	// create new area
	  } else {
		  naddr = UPP::uHeapManager::memalignNoStats( oalign, size ); // create new aligned area
	  } // if

	  UPP::uHeapManager::heapManagerInstance->headers( "realloc", naddr, header, freeElem, bsize, oalign );
	  // To preserve prior fill, the entire bucket must be copied versus the size.
	  memcpy( naddr, oaddr, std::min( osize, size ) );	// copy bytes
	  free( oaddr );

	  if ( UNLIKELY( ozfill ) ) {						// previous request zero fill ?
		  header->kind.real.blockSize |= 2;				// mark new request as zero filled
		  if ( size > osize ) {							// previous request larger ?
			  memset( (char *)naddr + osize, '\0', size - osize ); // initialize added storage
		  } // if
	  } // if
	  return naddr;
	} // realloc


	// Same as malloc() except the memory address is a multiple of alignment, which must be a power of two. (obsolete)
	void * memalign( size_t alignment, size_t size ) __THROW {
		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::memalign_calls, 1 );
		uFetchAdd( UPP::uHeapManager::memalign_storage, size );
		#endif // __U_STATISTICS__

		return UPP::uHeapManager::memalignNoStats( alignment, size );
	} // memalign


	// Same as aalloc() with memory alignment.
	void * amemalign( size_t alignment, size_t dim, size_t elemSize ) __THROW  {
		size_t size = dim * elemSize;
		#ifdef __STATISTICS__
		uFetchAdd( UPP::uHeapManager::cmemalign_calls, 1 );
		uFetchAdd( UPP::uHeapManager::cmemalign_storage, size );
		#endif // __STATISTICS__

		return UPP::uHeapManager::memalignNoStats( alignment, size );
	} // amemalign


	// Same as calloc() with memory alignment.
	void * cmemalign( size_t alignment, size_t dim, size_t elemSize ) __THROW {
		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::cmemalign_calls, 1 );
		uFetchAdd( UPP::uHeapManager::cmemalign_storage, dim * elemSize );
		#endif // __U_STATISTICS__

		return UPP::uHeapManager::cmemalignNoStats( alignment, dim, elemSize );
	} // cmemalign


	// Same as memalign(), but ISO/IEC 2011 C11 Section 7.22.2 states: the value of size shall be an integral multiple
    // of alignment. This requirement is universally ignored.
	void * aligned_alloc( size_t alignment, size_t size ) {
		return memalign( alignment, size );
	} // aligned_alloc


	// Allocates size bytes and places the address of the allocated memory in *memptr. The address of the allocated
	// memory shall be a multiple of alignment, which must be a power of two and a multiple of sizeof(void *). If size
	// is 0, then posix_memalign() returns either nullptr, or a unique pointer value that can later be successfully passed to
	// free(3).
	int posix_memalign( void ** memptr, size_t alignment, size_t size ) {
	  if ( alignment < uAlign() || ! uPow2( alignment ) ) return EINVAL; // check alignment
		* memptr = memalign( alignment, size );
		return 0;
	} // posix_memalign


	// Allocates size bytes and returns a pointer to the allocated memory. The memory address shall be a multiple of the
	// page size.  It is equivalent to memalign(sysconf(_SC_PAGESIZE),size).
	void * valloc( size_t size ) __THROW {
		return memalign( UPP::uHeapManager::pageSize, size );
	} // valloc


	// Same as valloc but rounds size to multiple of page size.
	void * pvalloc( size_t size ) __THROW {				// round size to multiple of page size
		return memalign( UPP::uHeapManager::pageSize, uCeiling( size, UPP::uHeapManager::pageSize ) );
	} // pvalloc


	// Frees the memory space pointed to by ptr, which must have been returned by a previous call to malloc(), calloc()
	// or realloc().  Otherwise, or if free(ptr) has already been called before, undefined behaviour occurs. If ptr is
	// nullptr, no operation is performed.
	void free( void * addr ) __THROW {
		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::free_calls, 1 );
		#endif // __U_STATISTICS__

	  if ( UNLIKELY( addr == nullptr ) ) {				// special case
			#ifdef __U_PROFILER__
			if ( uThisTask().profileActive && uProfiler::uProfiler_registerMemoryDeallocate ) {
				(* uProfiler::uProfiler_registerMemoryDeallocate)( uProfiler::profilerInstance, addr, 0, 0 ); 
			} // if
			#endif // __U_PROFILER__
			// #ifdef __U_DEBUG__
			// if ( UPP::uHeapControl::traceHeap() ) {
			// 	#define nullmsg "Free( 0x0 ) size:0\n"
			// 	// Do not debug print free( nullptr ), as it can cause recursive entry from sprintf.
			// 	uDebugWrite( STDERR_FILENO, nullmsg, sizeof(nullmsg) - 1 );
			// } // if
			// #endif // __U_DEBUG__
			return;
		} // exit

		UPP::uHeapManager::heapManagerInstance->doFree( addr );
		// Do not debug print free( nullptr ), as it can cause recursive entry from sprintf.
		uDEBUGPRT( uDebugPrt( "free( %p )\n", addr ); )
	} // free


	// Returns the alignment of an allocation.
	size_t malloc_alignment( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) return uAlign(); // minimum alignment
		UPP::uHeapManager::Storage::Header * header = headerAddr( addr );
		if ( (header->kind.fake.alignment & 1) == 1 ) {	// fake header ?
			return header->kind.fake.alignment & -2;	// remove flag from value
		} else {
			return uAlign();							// minimum alignment
		} // if
	} // malloc_alignment

	// Returns true if the allocation is zero filled, e.g., allocated by calloc().
	bool malloc_zero_fill( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) return false; // null allocation is not zero fill
		UPP::uHeapManager::Storage::Header * header = headerAddr( addr );
		if ( (header->kind.fake.alignment & 1) == 1 ) { // fake header ?
			header = realHeader( header );				// backup from fake to real header
		} // if
		return (header->kind.real.blockSize & 2) != 0;	// zero filled ?
	} // malloc_zero_fill


	// Returns original total allocation size (not bucket size) => array size is dimension * sizeif(T).
	size_t malloc_size( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) return 0; // null allocation is not zero fill
		UPP::uHeapManager::Storage::Header * header = headerAddr( addr );
		if ( (header->kind.fake.alignment & 1) == 1 ) { // fake header ?
			header = realHeader( header );				// backup from fake to real header
		} // if
		return header->kind.real.size;
	} // malloc_size


	// Returns the number of usable bytes in the block pointed to by ptr, a pointer to a block of memory allocated by
	// malloc or a related function.
	size_t malloc_usable_size( void * addr ) __THROW {
	  if ( UNLIKELY( addr == nullptr ) ) return 0;		// null allocation has 0 size
		UPP::uHeapManager::Storage::Header * header;
		UPP::uHeapManager::FreeHeader * freeElem;
		size_t bsize, alignment;

		UPP::uHeapManager::heapManagerInstance->headers( "malloc_usable_size", addr, header, freeElem, bsize, alignment );
		return dataStorage( bsize, addr, header );		// data storage in bucket
	} // malloc_usable_size


	// Prints (on default standard error) statistics about memory allocated by malloc and related functions.
	void malloc_stats() __THROW {
		#ifdef __U_STATISTICS__
		UPP::uHeapManager::printStats();
		if ( UPP::uHeapControl::prtFree() ) UPP::uHeapManager::heapManagerInstance->prtFree();
		#endif // __U_STATISTICS__
	} // malloc_stats


	// Changes the file descripter where malloc_stats() writes statistics.
	int malloc_stats_fd( int fd __attribute__(( unused )) ) __THROW {
		#ifdef __U_STATISTICS__
		int temp = UPP::uHeapManager::stats_fd;
		UPP::uHeapManager::stats_fd = fd;
		return temp;
		#else
		return -1;
		#endif // __U_STATISTICS__
	} // malloc_stats_fd


	// Adjusts parameters that control the behaviour of the memory-allocation functions (see malloc). The param argument
	// specifies the parameter to be modified, and value specifies the new value for that parameter.
	int mallopt( int option, int value ) __THROW {
		switch( option ) {
		  case M_TOP_PAD:
			UPP::uHeapManager::heapExpand = uCeiling( value, UPP::uHeapManager::pageSize ); return 1;
		  case M_MMAP_THRESHOLD:
			if ( UPP::uHeapManager::heapManagerInstance->setMmapStart( value ) ) return 1;
			break;
		} // switch
		return 0;										// error, unsupported
	} // mallopt


	// Attempt to release free memory at the top of the heap (by calling sbrk with a suitable argument).
	int malloc_trim( size_t ) {
		return 0;										// => impossible to release memory
	} // malloc_trim


	// Exports an XML string that describes the current state of the memory-allocation implementation in the caller.
	// The string is printed on the file stream stream.  The exported string includes information about all arenas (see
	// malloc).
	int malloc_info( int options, FILE * stream ) {
	  if ( options != 0 ) { errno = EINVAL; return -1; }
		#ifdef __U_STATISTICS__
		return UPP::uHeapManager::printStatsXML( stream );
		#else
		return 0;										// unsupported
		#endif // __U_STATISTICS__
	} // malloc_info


	// Records the current state of all malloc internal bookkeeping variables (but not the actual contents of the heap
	// or the state of malloc_hook functions pointers).  The state is recorded in a system-dependent opaque data
	// structure dynamically allocated via malloc, and a pointer to that data structure is returned as the function
	// result.  (The caller must free this memory.)
	void * malloc_get_state( void ) {
		return nullptr;									// unsupported
	} // malloc_get_state


	// Restores the state of all malloc internal bookkeeping variables to the values recorded in the opaque data
	// structure pointed to by state.
	int malloc_set_state( void * ) {
		return 0;										// unsupported
	} // malloc_set_state
} // extern "C"


// Must have C++ linkage to overload with C linkage realloc.
void * resize( void * oaddr, size_t nalign, size_t size ) __THROW {
	#ifdef __STATISTICS__
	uFetchAdd( UPP::uHeapManager::resize_calls, 1 );
	#endif // __STATISTICS__

	if ( UNLIKELY( nalign < uAlign() ) ) nalign = uAlign(); // reset alignment to minimum
	#ifdef __CFA_DEBUG__
	else
		checkAlign( nalign );							// check alignment
	#endif // __CFA_DEBUG__

	// If size is equal to 0, either NULL or a pointer suitable to be passed to free() is returned.
  if ( UNLIKELY( size == 0 ) ) { free( oaddr ); return nullptr; } // special cases
  if ( UNLIKELY( oaddr == nullptr ) ) {
		#ifdef __STATISTICS__
		uFetchAdd( UPP::uHeapManager::resize_storage, size );
		#endif // __STATISTICS__
		return UPP::uHeapManager::memalignNoStats( nalign, size );
	} // if

	// Attempt to reuse existing alignment.
	UPP::uHeapManager::Storage::Header * header = headerAddr( oaddr );
	bool isFakeHeader = header->kind.fake.alignment & 1; // old fake header ?
	size_t oalign;
	if ( isFakeHeader ) {
		oalign = header->kind.fake.alignment & -2;		// old alignment
		if ( (uintptr_t)oaddr % nalign == 0				// lucky match ?
			 && ( oalign <= nalign						// going down
				  || (oalign >= nalign && oalign <= 256) ) // little alignment storage wasted ?
			) {
			headerAddr( oaddr )->kind.fake.alignment = nalign | 1; // update alignment (could be the same)
			UPP::uHeapManager::FreeHeader * freeElem;
			size_t bsize, oalign;
			UPP::uHeapManager::heapManagerInstance->headers( "resize", oaddr, header, freeElem, bsize, oalign );
			size_t odsize = dataStorage( bsize, oaddr, header ); // data storage available in bucket

			if ( size <= odsize && odsize <= size * 2 ) { // allow 50% wasted data storage
				headerAddr( oaddr )->kind.fake.alignment = nalign | 1; // update alignment (could be the same)

				header->kind.real.blockSize &= -2;		// turn off 0 fill
				header->kind.real.size = size;			// reset allocation size
				return oaddr;
			} // if
		} // if
	} else if ( ! isFakeHeader							// old real header (aligned on libAlign) ?
				&& nalign == uAlign() ) {				// new alignment also on libAlign => no fake header needed
		return resize( oaddr, size );					// duplicate special case checks
	} // if

	#ifdef __STATISTICS__
	uFetchAdd( UPP::uHeapManager::resize_storage, size );
	#endif // __STATISTICS__

	// change size, DO NOT preserve STICKY PROPERTIES.
	free( oaddr );
	return UPP::uHeapManager::memalignNoStats( nalign, size ); // create new aligned area
} // resize


void * realloc( void * oaddr, size_t nalign, size_t size ) __THROW {
	if ( UNLIKELY( nalign < uAlign() ) ) nalign = uAlign(); // reset alignment to minimum
	#ifdef __U_DEBUG__
	else
		UPP::uHeapManager::checkAlign( nalign );		// check alignment
	#endif // __U_DEBUG__

	// If size is equal to 0, either NULL or a pointer suitable to be passed to free() is returned.
  if ( UNLIKELY( size == 0 ) ) { free( oaddr ); return nullptr; } // special cases
  if ( UNLIKELY( oaddr == nullptr ) ) {
		#ifdef __U_STATISTICS__
		uFetchAdd( UPP::uHeapManager::realloc_calls, 1 );
		uFetchAdd( UPP::uHeapManager::realloc_storage, size );
		#endif // __U_STATISTICS__
		return UPP::uHeapManager::memalignNoStats( nalign, size );
	} // if

	UPP::uHeapManager::Storage::Header * header = headerAddr( oaddr );
	bool isFakeHeader = header->kind.fake.alignment & 1; // old fake header ?
	size_t oalign;
	if ( isFakeHeader ) {
		oalign = header->kind.fake.alignment & -2;		// old alignment
		if ( (uintptr_t)oaddr % nalign == 0				// lucky match ?
			 && ( oalign <= nalign						// going down
				  || (oalign >= nalign && oalign <= 256) ) // little alignment storage wasted ?
			) {
			headerAddr( oaddr )->kind.fake.alignment = nalign | 1; // update alignment (could be the same)
			return realloc( oaddr, size );				// duplicate alignment and special case checks
		} // if
	} else if ( ! isFakeHeader							// old real header (aligned on libAlign) ?
				&& nalign == uAlign() )					// new alignment also on libAlign => no fake header needed
		return realloc( oaddr, size );					// duplicate alignment and special case checks

	#ifdef __U_STATISTICS__
	uFetchAdd( UPP::uHeapManager::realloc_calls, 1 );
	uFetchAdd( UPP::uHeapManager::realloc_storage, size );
	#endif // __U_STATISTICS__

	UPP::uHeapManager::FreeHeader * freeElem;
	size_t bsize;
	UPP::uHeapManager::heapManagerInstance->headers( "realloc", oaddr, header, freeElem, bsize, oalign );

	// change size and copy old content to new storage

	size_t osize = header->kind.real.size;				// old allocation size
	bool ozfill = (header->kind.real.blockSize & 2);	// old allocation zero filled

	void * naddr = UPP::uHeapManager::memalignNoStats( nalign, size ); // create new aligned area

	UPP::uHeapManager::heapManagerInstance->headers( "realloc", naddr, header, freeElem, bsize, oalign );
	memcpy( naddr, oaddr, std::min( osize, size ) );	// copy bytes
	free( oaddr );

	if ( UNLIKELY( ozfill ) ) {							// previous request zero fill ?
		header->kind.real.blockSize |= 2;				// mark new request as zero filled
		if ( size > osize ) {							// previous request larger ?
			memset( (char *)naddr + osize, '\0', size - osize ); // initialize added storage
		} // if
	} // if
	return naddr;
} // realloc


// Local Variables: //
// tab-width: 4 //
// compile-command: "make install" //
// End: //
