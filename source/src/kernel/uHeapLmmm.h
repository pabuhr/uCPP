//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uHeapLmmm.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Wed Jul 20 00:07:05 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Jul 19 16:40:00 2019
// Update Count     : 387
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


#define FASTLOOKUP

#define SPINLOCK 0
#define LOCKFREE 1
#define BUCKETLOCK SPINLOCK
#if BUCKETLOCK == LOCKFREE
#include <uStackLF.h>
#endif // LOCKFREE


extern "C" {
    void * malloc( size_t size ) __THROW;
    void * calloc( size_t noOfElems, size_t elemSize ) __THROW;
    void * realloc( void * addr, size_t size ) __THROW;
    void * memalign( size_t alignment, size_t size ) __THROW;
    void * valloc( size_t size ) __THROW;
    void free( void * addr ) __THROW;
    size_t malloc_alignment( void * addr ) __THROW;
    bool malloc_zero_fill( void * addr ) __THROW;
    size_t malloc_usable_size( void * addr ) __THROW;
    void malloc_stats() __THROW;
    int malloc_stats_fd( int fd ) __THROW;
    int mallopt( int param_number, int value ) __THROW;
} // extern "C"


namespace UPP {
    class uHeapManager {
	friend class uKernelBoot;			// access: uHeap
	friend class UPP::uMachContext;			// access: pageSize
	friend class UPP::uSigHandlerModule;		// access: print
	friend void * ::malloc( size_t size ) __THROW;	// access: boot
	friend void * ::calloc( size_t noOfElems, size_t elemSize ) __THROW;
	friend void * ::cmemalign( size_t alignment, size_t noOfElems, size_t elemSize ) __THROW; // access: Storage
	friend void * ::realloc( void * addr, size_t size ) __THROW; // access: boot
	friend void * ::memalign( size_t alignment, size_t size ) __THROW; // access: boot
	friend void * ::valloc( size_t size ) __THROW;	// access: pageSize
	friend void ::free( void * addr ) __THROW;	// access: doFree
	friend int ::mallopt( int param_number, int value ) __THROW; // access: heapManagerInstance, setHeapExpand, setMmapStart
	friend bool ::malloc_zero_fill( void * addr ) __THROW; // access: Storage
	// paraenthesis required for typedef
	friend size_t (::malloc_alignment)( void * addr ) __THROW; // access: Header, FreeHeader
	friend size_t (::malloc_usable_size)( void * addr ) __THROW; // access: Header, FreeHeader
	friend void ::malloc_stats() __THROW;
	friend int ::malloc_stats_fd( int fd ) __THROW;
	friend class uHeapControl;			// access: heapManagerInstance, boot
	#ifdef __U_STATISTICS__
	friend void UPP::Statistics::print();
	#endif // __U_STATISTICS__

	struct FreeHeader;				// forward declaration

	struct Storage {
	    struct Header {				// header
		union Kind {
		    struct RealHeader {
			union {
			    struct {			// 32-bit word => 64-bit header, 64-bit word => 128-bit header
				#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ && __U_WORDSIZE__ == 32
				uint32_t padding;	// unused, force home/blocksize to overlay alignment in fake header
				#endif // __ORDER_BIG_ENDIAN__ && __U_WORDSIZE__ == 32

				union {
				    FreeHeader * home;	// allocated block points back to home locations (must overlay alignment)
				    size_t blockSize;	// size for munmap (must overlay alignment)
				    #if BUCKLOCK == SPINLOCK
				    Storage * next;	// freed block points next freed block of same size
				    #endif // SPINLOCK
				};

				#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ && __U_WORDSIZE__ == 32
				    uint32_t padding;	// unused, force home/blocksize to overlay alignment in fake header
				#endif // __ORDER_LITTLE_ENDIAN__ && __U_WORDSIZE__ == 32
			    };
			    #if BUCKLOCK == LOCKFREE
			    Stack<Storage>::Link next;	// freed block points next freed block of same size (double-wide)
			    #endif // LOCKFREE
			};
		    } real;
		    struct FakeHeader {
			#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			uint32_t alignment;		// low-order bits of home/blockSize used for tricks
			#endif // __ORDER_LITTLE_ENDIAN__

			uint32_t offset;

			#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			uint32_t alignment;		// low-order bits of home/blockSize used for tricks
			#endif // __ORDER_BIG_ENDIAN__
		    } fake;
		} kind;
		#if defined( __U_PROFILER__ )
		// Used by uProfiler to find matching allocation data-structure for a deallocation.
		size_t * profileMallocEntry;
		#define PROFILEMALLOCENTRY( header ) ( header->profileMallocEntry )
		#endif // __U_PROFILER__
	    } header; // Header
	    char pad[uAlign() - sizeof( Header )];
	    char data[0];				// storage
	}; // Storage

	static_assert( uAlign() >= sizeof( Storage ), "uAlign() < sizeof( Storage )" );

	struct FreeHeader {
	    #if BUCKLOCK == SPINLOCK
	    uSpinLock lock;				// must be first field for alignment
	    Storage * freeList;
	    #elif BUCKLOCK == LOCKFREE
	    StackLF<Storage> freeList;
	    #else
		#error undefined lock type for bucket lock
	    #endif // SPINLOCK
	    size_t blockSize;				// size of allocations on this list

	    bool operator<( const size_t bsize ) const { return blockSize < bsize; }
	}; // FreeHeader

	enum { NoBucketSizes = 93,			// number of buckets sizes
	       #ifdef FASTLOOKUP
	       LookupSizes = 65536 + sizeof(uHeapManager::Storage), // number of fast lookup sizes
	       #endif // FASTLOOKUP
	};

	static uHeapManager * heapManagerInstance;	// pointer to heap manager object
	static size_t pageSize;				// architecture pagesize
	static size_t heapExpand;			// sbrk advance
	static size_t mmapStart;			// cross over point for mmap
	static unsigned int maxBucketsUsed;		// maximum number of buckets in use
	static unsigned int bucketSizes[NoBucketSizes];	// different bucket sizes
	#ifdef FASTLOOKUP
	static unsigned char lookup[LookupSizes];	// O(1) lookup for small sizes
	#endif // FASTLOOKUP
	static int mmapFd;				// fake or actual fd for anonymous file
	#ifdef __U_DEBUG__
	static unsigned long int allocfree;		// running total of allocations minus frees
	#endif // __U_DEBUG__

	#ifdef __U_STATISTICS__
	// Heap statistics
	static unsigned long long int mmap_storage;
	static unsigned int mmap_calls;
	static unsigned long long int munmap_storage;
	static unsigned int munmap_calls;
	static unsigned long long int sbrk_storage;
	static unsigned int sbrk_calls;
	static unsigned long long int malloc_storage;
	static unsigned int malloc_calls;
	static unsigned long long int free_storage;
	static unsigned int free_calls;
	static unsigned long long int calloc_storage;
	static unsigned int calloc_calls;
	static unsigned long long int memalign_storage;
	static unsigned int memalign_calls;
	static unsigned long long int cmemalign_storage;
	static unsigned int cmemalign_calls;
	static unsigned long long int realloc_storage;
	static unsigned int realloc_calls;
	static int stats_fd;
	static void print();
	#endif // __U_STATISTICS__

	// The next variables are statically allocated => zero filled.

	// must be first fields for alignment
	uSpinLock extlock;				// protects allocation-buffer extension
	FreeHeader freeLists[NoBucketSizes];		// buckets for different allocation sizes

	void * heapBegin;				// start of heap
	void * heapEnd;					// logical end of heap
	size_t heapRemaining;				// amount of storage not allocated in the current chunk

	static void boot();
	static void noMemory();				// called by "builtin_new" when malloc returns 0
	static void fakeHeader( Storage::Header *& header, size_t & alignment );
	static void checkAlign( size_t alignment );
	static bool setHeapExpand( size_t value );
	static bool setMmapStart( size_t value );

	bool headers( const char * name, void * addr, Storage::Header *& header, FreeHeader *& freeElem, size_t & size, size_t & alignment );
	void * extend( size_t size );
	void * doMalloc( size_t size );
	static void * mallocNoStats( size_t size ) __THROW;
	static void * memalignNoStats( size_t alignment, size_t size ) __THROW;
	void doFree( void * addr );
	size_t prtFree();
	uHeapManager();
	~uHeapManager();

	void * operator new( size_t, void * storage );
	void * operator new( size_t size );
      public:
    }; // uHeapManager
} // UPP


// Local Variables: //
// compile-command: "make install" //
// End: //
