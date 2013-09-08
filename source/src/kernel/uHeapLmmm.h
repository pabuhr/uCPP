//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uHeap.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Wed Jul 20 00:07:05 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Aug 13 14:47:51 2011
// Update Count     : 251
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


#ifndef __U_HEAPMANAGER_H__
#define __U_HEAPMANAGER_H__


#define FASTLOOKUP

class MMInfoEntry;					// for profiler

#if defined( __GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ > 2 || __GNUC__ == 4 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ >= 1)
#define uMallReturnType size_t
#else
// Old gcc cannot handle typedef for size_t and global :: qualifier in namespace
#if __U_WORDSIZE__ == 32
#define uMallReturnType unsigned int
#else
#define uMallReturnType unsigned long int
#endif
#endif

extern "C" void *malloc( size_t size ) __THROW;
extern "C" void *calloc( size_t noOfElems, size_t elemSize ) __THROW;
extern "C" void *realloc( void *addr, size_t size ) __THROW;
extern "C" void *memalign( size_t alignment, size_t size ) __THROW;
extern "C" void *valloc( size_t size ) __THROW;
extern "C" void free( void *addr ) __THROW;
extern "C" uMallReturnType malloc_alignment( void *addr ) __THROW;
extern "C" bool malloc_zero_fill( void *addr ) __THROW;
extern "C" uMallReturnType malloc_usable_size( void *addr ) __THROW;
extern "C" void malloc_stats() __THROW;
extern "C" int malloc_stats_fd( int fd ) __THROW;
extern "C" int mallopt( int param_number, int value ) __THROW;


namespace UPP {
    class uHeapManager {
	friend class uKernelBoot;			// access: uHeap
	friend class UPP::uMachContext;			// access: pageSize
	friend class UPP::uSigHandlerModule;		// access: print
	friend void *::malloc( size_t size ) __THROW;	// access: boot
	friend void *::calloc( size_t noOfElems, size_t elemSize ) __THROW;
	friend void *::cmemalign( size_t alignment, size_t noOfElems, size_t elemSize ) __THROW; // access: Storage
	friend void *::realloc( void *addr, size_t size ) __THROW; // access: boot
	friend void *::memalign( size_t alignment, size_t size ) __THROW; // access: boot
	friend void *::valloc( size_t size ) __THROW;	// access: pageSize
	friend void ::free( void *addr ) __THROW;	// access: doFree
	friend int ::mallopt( int param_number, int value ) __THROW; // access: heapManagerInstance, setHeapExpand, setMmapStart
	friend uMallReturnType ::malloc_alignment( void *addr ) __THROW; // access: Header, FreeHeader
	friend bool ::malloc_zero_fill( void *addr ) __THROW; // access: Storage
	friend uMallReturnType ::malloc_usable_size( void *addr ) __THROW; // access: Header, FreeHeader
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
#if __U_WORDSIZE__ == 32
#ifdef __U_PROFILER__
			// Used by uProfiler to find matching allocation data-structure for a deallocation.
			MMInfoEntry *profileMallocEntry;
#define			PROFILEMALLOCENTRY( header ) ( header->kind.real.profileMallocEntry )
#else
			uint32_t padding;		// unused
#endif // __U_PROFILER__
#endif // __U_WORDSIZE__ == 32
			union {
			    FreeHeader *home;		// allocated block points back to home locations
			    size_t blockSize;		// size for munmap
			    Storage *next;		// freed block points next freed block of same size
			};
		    } real;
		    struct FakeHeader {
			uint32_t offset, alignment;
		    } fake;
		} kind;
#if __U_WORDSIZE__ == 64 && defined( __U_PROFILER__ )
		// Used by uProfiler to find matching allocation data-structure for a deallocation.
		MMInfoEntry *profileMallocEntry;
#define		PROFILEMALLOCENTRY( header ) ( header->profileMallocEntry )
#endif // __U_WORDSIZE__ == 64 && defined( __U_PROFILER__ )
	    } header; // Header
	    char data[0];				// storage
	}; // Storage

	struct FreeHeader {
	    uSpinLock lock;				// must be first field for alignment
	    size_t blockSize;				// size of allocations on this list
	    Storage *freeList;

	    bool operator<( const FreeHeader &a2 ) const { return blockSize < a2.blockSize; }
	}; // FreeHeader

	enum { NoBucketSizes = 97,			// number of buckets sizes
#ifdef FASTLOOKUP
	       LookupSizes = 65536,			// number of fast lookup sizs
#endif // FASTLOOKUP
	};

	static uHeapManager *heapManagerInstance;	// pointer to heap manager object
	static size_t pageSize;				// architecture pagesize
	static size_t heapExpand;			// sbrk advance
	static size_t mmapStart;			// cross over point for mmap
	static unsigned int maxBucketsUsed;		// maximum number of buckets in use
	static unsigned int bucketSizes[NoBucketSizes];	// different bucket sizes
#ifdef FASTLOOKUP
	static char lookup[LookupSizes];		// O(1) lookup for small sizes
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
	static int statfd;
	static void print();
#endif // __U_STATISTICS__

	// The next variables are statically allocated => zero filled.

	// must be first fields for alignment
	FreeHeader freeLists[NoBucketSizes];		// buckets for different allocation sizes
	uSpinLock extlock;				// protects allocation-buffer extension

	void *heapBegin;				// start of heap
	void *heapEnd;					// logical end of heap
	size_t heapRemaining;				// amount of storage not allocated in the current chunk

	static void boot();
	static void noMemory();				// called by "builtin_new" when malloc returns 0
	static void checkAlign( size_t alignment );
	static bool setHeapExpand( size_t value );
	static bool setMmapStart( size_t value );

	bool headers( const char *name, void *addr, Storage::Header *&header, FreeHeader *&freeElem, size_t &size, size_t &alignment );
	void *extend( size_t size );
	void *doMalloc( size_t size );
	void doFree( void *addr );
	size_t checkFree( bool prt = false );
	uHeapManager();
	~uHeapManager();

	void *operator new( size_t, void *storage );
	void *operator new( size_t size );
      public:
    }; // uHeapManager
} // UPP


#endif // __U_HEAPMANAGER_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
