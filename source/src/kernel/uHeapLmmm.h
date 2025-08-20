// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uHeapLmmm.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Wed Jul 20 00:07:05 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Aug 15 18:39:02 2025
// Update Count     : 554
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

#include <malloc.h>

extern "C" {
	// New allocation operations
	void * aalloc( size_t dimension, size_t elemSize ) __THROW __attribute_warn_unused_result__ __attribute__ ((malloc)) __attribute_alloc_size__ ((1, 2)); // calloc - zero-fill
	void * resize( void * oaddr, size_t size ) __THROW __attribute_warn_unused_result__ __attribute_alloc_size__ ((2)); // realloc - data copy
	void * resizearray( void * oaddr, size_t dimension, size_t elemSize ) __THROW __attribute_warn_unused_result__ __attribute_alloc_size__ ((2, 3)); // reallocarray - data copy
	int posix_realloc( void ** oaddrp, size_t size ) __THROW;
	int posix_reallocarray( void ** oaddrp, size_t dimension, size_t elemSize ) __THROW;
	void * amemalign( size_t alignment, size_t dimension, size_t elemSize ) __THROW __attribute_warn_unused_result__ __attribute__ ((malloc)) __attribute_alloc_size__ ((2, 3)); // memalign + array
	void * cmemalign( size_t alignment, size_t dimension, size_t elemSize ) __THROW __attribute_warn_unused_result__ __attribute__ ((malloc)) __attribute_alloc_size__ ((2, 3)); // memalign + zero-fil
	void * aligned_resize( void * oaddr, size_t nalignment, size_t size ) __THROW __attribute_warn_unused_result__ __attribute_alloc_size__ ((3)); // resize + alignment
	void * aligned_resizearray( void * oaddr, size_t nalignment, size_t dimension, size_t elemSize ) __THROW __attribute_warn_unused_result__ __attribute_alloc_size__ ((3, 4)); // resizearray + alignment
	void * aligned_realloc( void * oaddr, size_t nalignment, size_t size ) __THROW __attribute_warn_unused_result__ __attribute_alloc_size__ ((3)); // realloc + alignment
	void * aligned_reallocarray( void * oaddr, size_t nalignment, size_t dimension, size_t elemSize ) __THROW __attribute_warn_unused_result__ __attribute_alloc_size__ ((3, 4)); // reallocarray + alignment
	int posix_aligned_realloc( void ** oaddrp, size_t nalignment, size_t size ) __THROW;
	int posix_aligned_reallocarray( void ** oaddrp, size_t nalignment, size_t dimension, size_t elemSize ) __THROW;

	// New control operations
	size_t malloc_extend( void );						// heap extend size (bytes)
	size_t malloc_mmap_start( void );					// crossover allocation size from sbrk to mmap
	size_t malloc_unfreed( void );						// amount subtracted to adjust for unfreed program storage (debug only)

	// Preserved properties
	size_t malloc_size( void * addr ) __THROW __attribute_warn_unused_result__;		 // object's request size, malloc_size <= malloc_usable_size
	size_t malloc_alignment( void * addr ) __THROW __attribute_warn_unused_result__; // object alignment
	bool malloc_zero_fill( void * addr ) __THROW __attribute_warn_unused_result__;	 // true if object is zero filled
	bool malloc_remote( void * addr ) __THROW __attribute_warn_unused_result__;		 // true if object is remote

	// Statistics
	int malloc_stats_fd( int fd ) __THROW;				// file descriptor global malloc_stats() writes (default stdout)
	void malloc_stats_clear( void ) __THROW;			// clear global heap statistics
	void heap_stats( void ) __THROW;					// print thread per heap statistics

	// If unsupport, create them, as supported in mallopt.
	#ifndef M_MMAP_THRESHOLD
	#define M_MMAP_THRESHOLD (-1)
	#endif // M_MMAP_THRESHOLD

	#ifndef M_TOP_PAD
	#define M_TOP_PAD (-2)
	#endif // M_TOP_PAD

	// Unsupported
	int malloc_trim( size_t ) __THROW;
	void * malloc_get_state( void ) __THROW;
	int malloc_set_state( void * ) __THROW;
} // extern "C"

// Local Variables: //
// mode: c++ //
// tab-width: 4 //
// compile-command: "make install" //
// End: //
