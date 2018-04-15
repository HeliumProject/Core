#include "Precompile.h"
#include "MemoryHeap.h"

#include "Platform/Atomic.h"
#include "Platform/Exception.h"
#include "Platform/Locks.h"
#include "Platform/Thread.h"
#include "Platform/Trace.h"
#include "Platform/Types.h"

using namespace Helium;

#if HELIUM_HEAP

#ifdef HELIUM_CC_CL
#pragma warning( push )
#pragma warning( disable : 4530 )  // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#endif

#include <map>

#ifdef HELIUM_CC_CL
#pragma warning( pop )
#endif

#if HELIUM_ENABLE_MEMORY_TRACKING
volatile size_t VirtualMemory::sm_bytesAllocated = 0;
#endif


/// Mutex wrapper buffer.  We use this buffer to be able to set aside space for an uninitialized Helium::Mutex in which
/// we can properly construct one using placement new, as dlmalloc/nedmalloc are built to deal with more basic C types.
struct MallocMutex
{
	size_t buffer[ ( sizeof( Helium::Mutex ) + sizeof( size_t ) - 1 ) / sizeof( size_t ) ];
};

/// Mutex initialize wrapper.
///
/// @param[in] pMutex  Mutex to initialize.
///
/// @return  Zero.
static int MallocMutexInitialize( MallocMutex* pMutex )
{
	new( pMutex->buffer ) Helium::Mutex;
	return 0;
}

/// Mutex lock wrapper.
///
/// @param[in] pMutex  Mutex to lock.
///
/// @return  Zero.
static int MallocMutexLock( MallocMutex* pMutex )
{
	reinterpret_cast< Helium::Mutex* >( pMutex->buffer )->Lock();
	return 0;
}

/// Mutex unlock wrapper.
///
/// @param[in] pMutex  Mutex to unlock.
///
/// @return  Zero.
static int MallocMutexUnlock( MallocMutex* pMutex )
{
	reinterpret_cast< Helium::Mutex* >( pMutex->buffer )->Unlock();
	return 0;
}

/// Mutex try-lock wrapper.
///
/// @param[in] pMutex  Mutex to attempt to lock.
///
/// @return  True if the mutex was locked successfully by this thread, false if it was locked by another thread.
static bool MallocMutexTryLock( MallocMutex* pMutex )
{
	return reinterpret_cast< Helium::Mutex* >( pMutex->buffer )->TryLock();
}

/// Get a reference to the global dlmalloc mutex.
///
/// @return  Reference to the global dlmalloc mutex.
static Helium::Mutex& GetMallocGlobalMutex()
{
	// Initialize as a local variable to try to ensure it is initialized the first time it is used.
	static Helium::Mutex globalMutex;
	return globalMutex;
}

#if HELIUM_ENABLE_MEMORY_TRACKING_VERBOSE
/// Dynamic memory heap verbose tracking data.
struct Helium::DynamicMemoryHeapVerboseTrackingData
{
	/// Allocation backtraces for this heap.
	std::map< void*, DynamicMemoryHeap::AllocationBacktrace > allocationBacktraceMap;
};

static volatile ThreadId s_verboseTrackingCurrentThreadId = Helium::InvalidThreadId;

static Mutex& GetVerboseTrackingMutex()
{
	static Mutex verboseTrackingMutex;

	return verboseTrackingMutex;
}

static bool ConditionalVerboseTrackingLock()
{
	ThreadId threadId = Thread::GetCurrentId();
	if( s_verboseTrackingCurrentThreadId != threadId )
	{
		GetVerboseTrackingMutex().Lock();
		s_verboseTrackingCurrentThreadId = threadId;

		return true;
	}

	return false;
}

static void VerboseTrackingUnlock()
{
	HELIUM_ASSERT( s_verboseTrackingCurrentThreadId == Thread::GetCurrentId() );

	s_verboseTrackingCurrentThreadId = Helium::InvalidThreadId;
	GetVerboseTrackingMutex().Unlock();
}
#endif  // HELIUM_ENABLE_MEMORY_TRACKING_VERBOSE

#ifndef USE_NEDMALLOC
#define USE_NEDMALLOC 0
#endif

#define MEMORY_HEAP_CLASS_NAME DynamicMemoryHeap

#define USE_LOCKS 2
#define MLOCK_T MallocMutex
#define CURRENT_THREAD Helium::Thread::GetCurrentId()
#define INITIAL_LOCK( sl ) MallocMutexInitialize( sl )
#define ACQUIRE_LOCK( sl ) MallocMutexLock( sl )
#define RELEASE_LOCK( sl ) MallocMutexUnlock( sl )
#define TRY_LOCK( sl ) MallocMutexTryLock( sl )
#define ACQUIRE_MALLOC_GLOBAL_LOCK() GetMallocGlobalMutex().Lock();
#define RELEASE_MALLOC_GLOBAL_LOCK() GetMallocGlobalMutex().Unlock();

#include "MemoryHeapImpl.inl"

DynamicMemoryHeap* volatile DynamicMemoryHeap::sm_pGlobalHeapListHead = NULL;
#if HELIUM_ENABLE_MEMORY_TRACKING_VERBOSE
volatile bool DynamicMemoryHeap::sm_bDisableBacktraceTracking = false;
#endif

/// Acquire a read-only lock on the global dynamic memory heap list.
///
/// @return  Heap at the head of the list.
///
/// @see UnlockReadGlobalHeapList(), GetPreviousHeap, GetNextHeap()
DynamicMemoryHeap* DynamicMemoryHeap::LockReadGlobalHeapList()
{
	GetGlobalHeapListLock().LockRead();

	return sm_pGlobalHeapListHead;
}

/// Release a previously acquired read-only lock on the global dynamic memory heap.
///
/// @see LockReadGlobalHeapList(), GetPreviousHeap, GetNextHeap()
void DynamicMemoryHeap::UnlockReadGlobalHeapList()
{
	GetGlobalHeapListLock().UnlockRead();
}

#if HELIUM_ENABLE_MEMORY_TRACKING
/// Write memory stats for all dynamic memory heap instances to the output log.
void DynamicMemoryHeap::LogMemoryStats()
{
	HELIUM_TRACE( TraceLevels::Debug, "DynamicMemoryHeap stats:\n" );
	HELIUM_TRACE( TraceLevels::Debug, "Heap name\tActive allocations\tBytes allocated\n" );

	ScopeReadLock readLock( GetGlobalHeapListLock() );

	for( DynamicMemoryHeap* pHeap = sm_pGlobalHeapListHead; pHeap != NULL; pHeap = pHeap->GetNextHeap() )
	{
		const char* pName = NULL;
#if !HELIUM_RELEASE && !HELIUM_PROFILE
		pName = pHeap->GetName();
#endif
		if( !pName )
		{
			pName = "(unnamed)";
		}

		size_t allocationCount = pHeap->GetAllocationCount();
		size_t bytesActual = pHeap->GetBytesActual();

		HELIUM_TRACE(
			TraceLevels::Debug,
			"%s\t%" PRIuSZ "\t%" PRIuSZ "\n",
			pName,
			allocationCount,
			bytesActual );
	}

	HELIUM_TRACE( TraceLevels::Debug, "\n" );

#if HELIUM_ENABLE_MEMORY_TRACKING_VERBOSE
	bool bLockedTracking = ConditionalVerboseTrackingLock();

	bool bOldDisableBacktraceTracking = sm_bDisableBacktraceTracking;
	sm_bDisableBacktraceTracking = true;

	HELIUM_TRACE( TraceLevels::Debug, "DynamicMemoryHeap unfreed allocations:\n" );

	size_t allocationIndex = 1;

	for( DynamicMemoryHeap* pHeap = sm_pGlobalHeapListHead; pHeap != NULL; pHeap = pHeap->GetNextHeap() )
	{
		const char* pHeapName = pHeap->GetName();

		DynamicMemoryHeapVerboseTrackingData* pTrackingData = pHeap->m_pVerboseTrackingData;
		if( pTrackingData )
		{
			const std::map< void*, AllocationBacktrace >& rAllocationBacktraceMap =
				pTrackingData->allocationBacktraceMap;

			// TODO: Remove STL string usage here once String is merged over
			std::string symbol;

			std::map< void*, AllocationBacktrace >::const_iterator iterEnd = rAllocationBacktraceMap.end();
			std::map< void*, AllocationBacktrace >::const_iterator iter;
			for( iter = rAllocationBacktraceMap.begin(); iter != iterEnd; ++iter )
			{
				HELIUM_TRACE(
					TraceLevels::Debug,
					"%" PRIuSZ ": 0x%" HELIUM_PRINT_POINTER " (%s)\n",
					allocationIndex,
					iter->first,
					pHeapName );
				++allocationIndex;

				void* const* ppTraceAddress = iter->second.pAddresses;
				for( size_t addressIndex = 0;
					addressIndex < HELIUM_ARRAY_COUNT( iter->second.pAddresses );
					++addressIndex )
				{
					void* pAddress = *ppTraceAddress;
					++ppTraceAddress;
					if( !pAddress )
					{
						break;
					}

					Helium::GetAddressSymbol( symbol, pAddress );
					const char* pSymbol = symbol.c_str();
					HELIUM_TRACE( TraceLevels::Debug, "- 0x%" HELIUM_PRINT_POINTER ": %s\n", pAddress, ( pSymbol ? pSymbol : "" ) );
				}
			}
		}
	}

	HELIUM_TRACE( TraceLevels::Debug, "\n" );

	sm_bDisableBacktraceTracking = bOldDisableBacktraceTracking;

	if( bLockedTracking )
	{
		VerboseTrackingUnlock();
	}
#endif
}
#endif  // HELIUM_ENABLE_MEMORY_TRACKING

/// Get the read-write lock used for synchronizing access to the global dynamic memory heap list.
///
/// @return  Global heap list read-write lock.
ReadWriteLock& DynamicMemoryHeap::GetGlobalHeapListLock()
{
	// Note that the construction of this is not inherently thread-safe, but we can be fairly certain that the main
	// thread will trigger the creation of the lock before any other threads are spawned.
	static ReadWriteLock globalHeapListLock;

	return globalHeapListLock;
}

#endif // HELIUM_HEAP

/// Constructor.
ThreadLocalStackAllocator::ThreadLocalStackAllocator()
{
	m_pHeap = &GetMemoryHeap();
	HELIUM_ASSERT( m_pHeap );
}

/// Allocate a block of memory.
///
/// @param[in] size  Number of bytes to allocate.
///
/// @return  Base address of the allocation if successful, null if allocation failed.
void* ThreadLocalStackAllocator::Allocate( size_t size )
{
	HELIUM_ASSERT( m_pHeap );
	void* pMemory = m_pHeap->Allocate( size );

	return pMemory;
}

/// Allocate a block of aligned memory.
///
/// @param[in] alignment  Allocation alignment (must be a power of two).
/// @param[in] size       Number of bytes to allocate.
///
/// @return  Base address of the allocation if successful, null if allocation failed.
void* ThreadLocalStackAllocator::AllocateAligned( size_t alignment, size_t size )
{
	HELIUM_ASSERT( m_pHeap );
	void* pMemory = m_pHeap->AllocateAligned( alignment, size );

	return pMemory;
}

/// Free a block of memory previously allocated using Allocate() or AllocateAligned().
///
/// @param[in] pMemory  Base address of the allocation to free.
void ThreadLocalStackAllocator::Free( void* pMemory )
{
	HELIUM_ASSERT( m_pHeap );
	m_pHeap->Free( pMemory );
}

/// Get the stack-based memory heap for the current thread.
///
/// @return  Reference to the current thread's stack-based heap.
///
/// @see ReleaseMemoryHeap()
StackMemoryHeap<>& ThreadLocalStackAllocator::GetMemoryHeap()
{
	ThreadLocalPointer& tls = GetMemoryHeapTls();
	StackMemoryHeap<>* pHeap = static_cast< StackMemoryHeap<>* >( tls.GetPointer() );
	if( !pHeap )
	{
		// Heap does not already exist for this thread, so create one.
		pHeap = new StackMemoryHeap<>( BLOCK_SIZE );
		HELIUM_ASSERT( pHeap );

		tls.SetPointer( pHeap );
	}

	return *pHeap;
}

/// Release the stack-based memory heap for the current thread.
///
/// This should be called by any thread using this allocator prior to exiting the thread in order to avoid leaking
/// memory.
///
/// @see GetMemoryHeap()
void ThreadLocalStackAllocator::ReleaseMemoryHeap()
{
	ThreadLocalPointer& tls = GetMemoryHeapTls();
	StackMemoryHeap<>* pHeap = static_cast< StackMemoryHeap<>* >( tls.GetPointer() );
	if( pHeap )
	{
		delete pHeap;
		tls.SetPointer( NULL );
	}
}

/// Get the thread-local storage pointer for the current thread's stack heap.
///
/// @return  Thread-local storage pointer for the thread-local stack heap instance.
ThreadLocalPointer& ThreadLocalStackAllocator::GetMemoryHeapTls()
{
	// Keeping this as a local static variable should help us enforce its construction on the first attempt to
	// allocate memory, regardless of whichever code attempts to dynamically allocate memory first.
	// XXX TMC: I'd imagine a race condition could still occur where multiple threads could use
	// ThreadLocalStackAllocator for the first time at around the same time.  To be safe, the main thread should try
	// to use ThreadLocalStackAllocator before any threads are created in order to prep this value (could do
	// something in the Thread class itself...).
	static ThreadLocalPointer memoryHeapTls;

	return memoryHeapTls;
}

#if HELIUM_HEAP

#if !HELIUM_USE_MODULE_HEAPS
/// Get the default heap used for dynamic allocations within the engine.
///
/// @return  Reference to the default dynamic memory heap.
DynamicMemoryHeap& Helium::GetDefaultHeap()
{
	static DynamicMemoryHeap* pDefaultHeap = NULL;
	if( !pDefaultHeap )
	{
		pDefaultHeap = static_cast< DynamicMemoryHeap* >( VirtualMemory::Allocate( sizeof( DynamicMemoryHeap ) ) );
		new( pDefaultHeap ) DynamicMemoryHeap HELIUM_DYNAMIC_MEMORY_HEAP_INIT( "Default" );
	}

	return *pDefaultHeap;
}
#endif

#if HELIUM_USE_EXTERNAL_HEAP
/// Get the fallback heap for allocations made by external libraries.
///
/// @return  Reference for the external allocation fallback heap.
DynamicMemoryHeap& Helium::GetExternalHeap()
{
	static DynamicMemoryHeap* pExternalHeap = NULL;
	if( !pExternalHeap )
	{
		pExternalHeap = static_cast< DynamicMemoryHeap* >( VirtualMemory::Allocate( sizeof( DynamicMemoryHeap ) ) );
		new( pExternalHeap ) DynamicMemoryHeap HELIUM_DYNAMIC_MEMORY_HEAP_INIT( "External" );
	}

	return *pExternalHeap;
}
#endif

#endif // HELIUM_HEAP