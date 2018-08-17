#include "Precompile.h"
#include "Thread.h"

#include "Platform/SystemWin.h"
#include "Platform/Assert.h"
#include "Platform/Error.h"
#include "Platform/Console.h"
#include "Platform/MemoryHeap.h"
#include "Platform/Encoding.h"

#include <process.h>

#undef Yield

using namespace Helium;

// Mapping from Helium::Thread priority values to Win32 thread priority values.
static const int WIN32_THREAD_PRIORITY_MAP[] =
{
	THREAD_PRIORITY_LOWEST,        // PRIORITY_LOWEST
	THREAD_PRIORITY_BELOW_NORMAL,  // PRIORITY_LOW
	THREAD_PRIORITY_NORMAL,        // PRIORITY_NORMAL
	THREAD_PRIORITY_ABOVE_NORMAL,  // PRIORITY_HIGH
	THREAD_PRIORITY_HIGHEST,       // PRIORITY_HIGHEST
};

const uint32_t Helium::InvalidThreadId = ~0;

/// Constructor.
///
/// @param[in] rName  Optional name to assign to the thread (for debugging purposes).
Thread::Thread()
	: m_Handle( 0 )
{
    m_Name[0] = char('\0');
}

/// Destructor.
Thread::~Thread()
{
	HELIUM_VERIFY( Join() );
}

/// Begin execution of this thread.
///
/// If a thread has been started and not yet joined, this will fail.
///
/// @param[in] priority  Thread priority.
///
/// @return  True if the thread was started successfully, false if not.
///
/// @see Join(), TryJoin()
bool Thread::Start( const char* pName, ThreadPriority priority )
{
	HELIUM_ASSERT( priority >= ThreadPriorities::Lowest && priority <= ThreadPriorities::Inherit );

	// Cache the name
	CopyString( m_Name, pName );

	// Make sure a thread hasn't already been started.
	HELIUM_ASSERT( m_Handle == 0 );
	if( m_Handle != 0 )
	{
		return false;
	}

	// Create the thread, but don't launch it yet (we want to set the thread name prior to the thread actually
	// starting).
	uint32_t threadId;
	m_Handle = (HANDLE)_beginthreadex( NULL, 0, ThreadCallback, this, CREATE_SUSPENDED, &threadId );
	HELIUM_ASSERT( m_Handle != 0 );
	if( m_Handle != 0 )
	{
		if ( priority != ThreadPriorities::Inherit )
		{
			// Set the thread priority.
			int win32Priority = WIN32_THREAD_PRIORITY_MAP[ priority ];
			BOOL priorityResult = SetThreadPriority( m_Handle, win32Priority );
			HELIUM_ASSERT( priorityResult );
			HELIUM_UNREF( priorityResult );
		}

		// Start the thread.
		DWORD resumeResult = ResumeThread( m_Handle );
		HELIUM_ASSERT( resumeResult != static_cast< DWORD >( -1 ) );
		HELIUM_UNREF( resumeResult );
	}

	return( m_Handle != 0 );
}

/// Wait for this thread to finish execution and release any allocated system resources.
///
/// @param[in] timeOutMilliseconds  Maximum time to wait for the thread to finish, or zero to wait indefinitely.
///
/// @return  True if the thread finished or was not running to begin with, false if it is still running.
bool Thread::Join( uint32_t timeOutMilliseconds )
{
	bool result = true;

	if( IsValid() )
	{
		DWORD waitResult = ::WaitForSingleObject( m_Handle, ( timeOutMilliseconds != 0 ? timeOutMilliseconds : INFINITE ) );
		HELIUM_ASSERT( waitResult == WAIT_OBJECT_0 || waitResult == WAIT_TIMEOUT );
		if( waitResult == WAIT_OBJECT_0 )
		{
			HELIUM_VERIFY( CloseHandle( m_Handle ) );
			m_Handle = 0;
		}
		else
		{
			result = false;
		}
	}

	if ( result )
	{
		m_Handle = 0;
	}

	return result;
}

/// Check if this thread has finished execution without blocking the calling thread.
///
/// If the thread has finished execution, any allocated system resources will be released.
///
/// @return  True if the thread finished or was not running to begin with, false if it is still running.
bool Thread::TryJoin()
{
	bool result = true;

	if ( IsValid() )
	{
		DWORD waitResult = ::WaitForSingleObject( m_Handle, 0 );
		HELIUM_ASSERT( waitResult == WAIT_OBJECT_0 || waitResult == WAIT_TIMEOUT );
		if( waitResult == WAIT_OBJECT_0 )
		{
			HELIUM_VERIFY( ::CloseHandle( m_Handle ) );
		}
		else
		{
			result = false;
		}
	}

	if ( result )
	{
		m_Handle = 0;
	}

	return false;
}

/// Get whether this thread is valid (created).
///
/// A thread is active if it has been successfully started using Start() but has not yet been joined using Join() or
/// TryJoin().
///
/// An active thread does not necessarily indicate active execution.  The thread may have completed work, but its
/// resources are not released until the thread is joined.  Manually managed status flags should be used by threads to
/// indicate active execution.
///
/// @return  True if this thread is active (started at some point but not yet joined), false if not.
bool Thread::IsValid() const
{
	return ( m_Handle != 0 );
}

/// @fn void Helium::Thread::Run()
/// Execute the thread code.

/// Suspend execution of the calling thread for at least a given period of time.
///
/// @param[in] milliseconds  Number of milliseconds to sleep.  Note that the calling thread may not be awaken
///                          immediately upon the timer expiring.
///
/// @see Yield()
void Thread::Sleep( uint32_t milliseconds )
{
	::Sleep( milliseconds );
}

void Thread::Yield()
{
	::Sleep( 0 );
}

ThreadId Thread::GetCurrentId()
{
	return ::GetCurrentThreadId();
}

static ThreadId g_MainThreadID = ::GetCurrentThreadId();

ThreadId Thread::GetMainId()
{
	return g_MainThreadID;
}

/// Thread callback function.
///
/// @param[in] pData  Callback data (in this case, the pointer to the Thread instance being run).
///
/// @return  Thread result code (always zero).
unsigned int __stdcall Thread::ThreadCallback( void* pData )
{
	HELIUM_ASSERT( pData );

	Thread* pThread = static_cast< Thread* >( pData );

	// Assign the thread name.
	if( pThread->m_Name && pThread->m_Name[ 0 ] != '\0' )
	{
		// Thread name assignment exception information.
		struct ThreadNameInfo
		{
			/// Info type (must be set to 0x1000).
			ULONG_PTR dwType;
			/// Thread name.
			LPCSTR szName;
			/// Thread ID (-1 = caller thread).
			ULONG_PTR dwThreadId;
			/// Reserved (must be set to zero).
			ULONG_PTR dwFlags;
		};

		ThreadNameInfo nameInfo;
		nameInfo.dwType = 0x1000;
		nameInfo.dwThreadId = ::GetCurrentThreadId();
		nameInfo.dwFlags = 0;
		nameInfo.szName = pThread->m_Name;

		__try
		{
			RaiseException(
				0x406D1388,
				0,
				sizeof( nameInfo ) / sizeof( ULONG_PTR ),
				reinterpret_cast< ULONG_PTR* >( &nameInfo ) );
		}
		__except( EXCEPTION_CONTINUE_EXECUTION )
		{
		}
	}

	pThread->Run();

	ThreadLocalStackAllocator::ReleaseMemoryHeap();

#if HELIUM_HEAP
	DynamicMemoryHeap::UnregisterCurrentThreadCache();
#endif

	_endthreadex( 0 );

	return 0;
}

ThreadLocalPointer::ThreadLocalPointer()
{
	m_Key = TlsAlloc(); 
	HELIUM_ASSERT(m_Key != TLS_OUT_OF_INDEXES);
	SetPointer(NULL); 
}

ThreadLocalPointer::~ThreadLocalPointer()
{
	TlsFree(m_Key); 
}

void* ThreadLocalPointer::GetPointer() const
{
	void* value = TlsGetValue(m_Key);
	return value;
}

void ThreadLocalPointer::SetPointer(void* pointer)
{
	TlsSetValue(m_Key, pointer); 
}
