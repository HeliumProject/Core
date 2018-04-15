#include "Precompile.h"
#include "MemoryHeap.h"

#if HELIUM_HEAP

#include "Platform/Atomic.h"

#include <unistd.h>
#include <sys/mman.h>

#if HELIUM_OS_LINUX
# include <linux/mman.h> // for MAP_UNINITIALIZED
#endif

using namespace Helium;

/// Allocate memory pages of at least the specified size.
///
/// Memory allocated using this interface can later be returned to the system using Free().
///
/// @param[in] size  Allocation size, in bytes.
///
/// @return  Address of the allocation if successfully allocated, null if not.
///
/// @see Free()
void* VirtualMemory::Allocate( size_t size )
{
	// different systems refer to the same flag by different macros
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
# define MAP_ANONYMOUS MAP_ANON
#endif

	int flags = MAP_PRIVATE | MAP_ANONYMOUS;

#if HELIUM_OS_LINUX && !HELIUM_DEBUG
	flags |= MAP_UNINITIALIZED;
#endif

	void* pMemory = mmap( NULL, size, PROT_READ | PROT_WRITE, flags, 0, 0 );
	HELIUM_ASSERT( pMemory != reinterpret_cast< void* >( ~static_cast< uintptr_t >( 0 ) ) );

#if HELIUM_ENABLE_MEMORY_TRACKING
	// This is a rough shot at computing the appropriate bytes in the number of pages allocated, it needs testing -geoff
	size_t pageSize = GetPageSize();
	size_t trackingSize = ( (size + pageSize) / pageSize ) * pageSize;
# if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
	size_t prev = __sync_fetch_and_add( static_cast< volatile size_t* >( &sm_bytesAllocated ), trackingSize );
# else
	size_t prev = __atomic_fetch_add( static_cast< volatile size_t* >( &sm_bytesAllocated ), trackingSize, __ATOMIC_SEQ_CST );
# endif
#endif

	return pMemory;
}

/// Free memory previously allocated using Allocate().
///
/// All memory pages allocated within the region starting at the given memory address up to the number of bytes
/// specified will be freed.  Multiple contiguous memory pages allocated via multiple contiguous calls to Allocate()
/// can be freed at once.
///
/// @param[in] pMemory  Base address of the allocation to free.
/// @param[in] size     Size of the range of pages to free.
///
/// @return  True if all pages were freed successfully, false if not.
///
/// @see Allocate()
bool VirtualMemory::Free( void* pMemory, size_t size )
{
	HELIUM_ASSERT( pMemory );
	HELIUM_ASSERT( size != 0 );

	uint8_t* pCurrentBase = static_cast< uint8_t* >( pMemory );
	HELIUM_ASSERT( pCurrentBase + size == 0 || pCurrentBase + size > pCurrentBase );  // Check address space bounds.

	int result = munmap( pMemory, size );
	HELIUM_ASSERT( result == 0 );

#if HELIUM_ENABLE_MEMORY_TRACKING
	// This is a rough shot at computing the appropriate bytes in the number of pages allocated, it needs testing -geoff
	size_t pageSize = GetPageSize();
	size_t trackingSize = ( (size + pageSize) / pageSize ) * pageSize;
# if !((__GNUC__ >= 4) && (__GNUC_MINOR__ >= 7))
	size_t prev = __sync_fetch_and_sub( static_cast< volatile size_t* >( &sm_bytesAllocated ), trackingSize );
# else
	size_t prev = __atomic_fetch_sub( static_cast< volatile size_t* >( &sm_bytesAllocated ), trackingSize, __ATOMIC_SEQ_CST );
# endif
#endif

	return result == 0;
}

/// Get the page size of memory allocated through this function.
///
/// @return  Current platform page size.
size_t VirtualMemory::GetPageSize()
{
	return sysconf(_SC_PAGE_SIZE);
}

#endif // HELIUM_HEAP