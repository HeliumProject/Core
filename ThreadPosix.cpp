#include "Platform/Thread.h"

#include "Platform/Assert.h"
#include "Platform/Exception.h"
#include "Platform/MemoryHeap.h"

#include <unistd.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/prctl.h>

using namespace Helium;

static void SetSchedParam(struct sched_param * params, ThreadPriority priority)
{
    HELIUM_ASSERT( priority != ThreadPriorities::Inherit );
    switch (priority)
    {
    case ThreadPriorities::Highest:
        params->sched_priority = 99;
        break;
    case ThreadPriorities::High:
        params->sched_priority = 75;
        break;
    case ThreadPriorities::Normal:
    default:
        params->sched_priority = 50;
        break;
    case ThreadPriorities::Low:
        params->sched_priority = 25;
        break;
    case ThreadPriorities::Lowest:
        params->sched_priority = 1;
        break;
    }
}

Thread::Thread()
    : m_Handle( 0 )
    , m_Valid( false )
{
    m_Name[0] = tchar_t('\0');
}

Thread::~Thread()
{
    if ( IsValid() )
    {
        HELIUM_VERIFY( this->Join() );
    }
}

bool Thread::Start( const tchar_t* pName, ThreadPriority priority )
{
    HELIUM_ASSERT( priority >= ThreadPriorities::Lowest && priority <= ThreadPriorities::Inherit );

    // Cache the name
    MemoryCopy( m_Name, pName, sizeof( m_Name ) / sizeof( tchar_t ) );

    pthread_attr_t attr;
    HELIUM_VERIFY( pthread_attr_init(&attr) == 0 );
    HELIUM_VERIFY( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) == 0 );
    if ( priority != ThreadPriorities::Inherit )
    {
        HELIUM_ASSERT( !Helium::IsDebuggerPresent() ); // following lines break gdb 7.5, see http://sourceware.org/bugzilla/show_bug.cgi?id=12203
        HELIUM_VERIFY( pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) == 0 );
        HELIUM_VERIFY( pthread_attr_setschedpolicy(&attr, SCHED_RR) == 0 );

        struct sched_param sched;
        MemorySet( &sched, 0, sizeof( sched ) );
        SetSchedParam(&sched, priority);
        HELIUM_VERIFY( pthread_attr_setschedparam(&attr, &sched) == 0 );
    }

    if ( pthread_create( &(this->m_Handle), &attr, ThreadCallback, this) == 0 )
    {
        m_Valid = true;
        prctl( PR_SET_NAME, reinterpret_cast< unsigned long >( pName ) );
    }

    HELIUM_VERIFY( pthread_attr_destroy(&attr) == 0);

    return IsValid();
}

bool Thread::Join( uint32_t timeOutMilliseconds )
{
    HELIUM_ASSERT( IsValid() );
    if ( IsValid() )
    {
        if ( timeOutMilliseconds )
        {
            struct timespec spec;
            spec.tv_sec = timeOutMilliseconds / 1000;
            spec.tv_nsec = ( timeOutMilliseconds % 1000 ) * 1000000;
            return pthread_timedjoin_np( m_Handle, NULL, &spec ) == 0;
        }
        else
        {
            return pthread_join( m_Handle, NULL ) == 0;
        }
    }
    else
    {
        return false;
    }
}

bool Thread::TryJoin()
{
    HELIUM_ASSERT( IsValid() );
    if ( IsValid() )
    {
        return pthread_tryjoin_np( m_Handle, NULL ) == 0;
    }
    else
    {
        return false;
    }
}

bool Thread::IsValid() const
{
    return m_Valid;
}

void Thread::Sleep( uint32_t milliseconds )
{
    usleep( milliseconds * 1000000 );
}

void Thread::Yield()
{
    sched_yield();
}

/// Get the ID of the thread in which this function is called.
///
/// @return  Current thread ID.
Thread::id_t Thread::GetCurrentId()
{
    return pthread_self();
}

void* Thread::ThreadCallback( void* pData )
{
    HELIUM_ASSERT( pData );

    Thread* pThread = static_cast< Thread* >( pData );

    prctl( PR_SET_NAME, reinterpret_cast< unsigned long >( pThread->m_Name ) );

    pThread->Run();

#if HELIUM_HEAP
    ThreadLocalStackAllocator::ReleaseMemoryHeap();
    DynamicMemoryHeap::UnregisterCurrentThreadCache();
#endif
    
    return 0;
}

ThreadLocalPointer::ThreadLocalPointer()
{
    int status = pthread_key_create(&m_Key, NULL);
    HELIUM_ASSERT( status == 0 && "Could not create pthread_key");
    SetPointer(NULL);
}

ThreadLocalPointer::~ThreadLocalPointer()
{
    pthread_key_delete(m_Key);
}

void* ThreadLocalPointer::GetPointer() const
{
    return pthread_getspecific(m_Key);
}

void ThreadLocalPointer::SetPointer(void* pointer)
{
    pthread_setspecific(m_Key, pointer);
}

static Thread::id_t g_MainThread = pthread_self();

uint32_t Helium::GetMainThreadID()
{
    return g_MainThread;
}

uint32_t Helium::GetCurrentThreadID()
{
    return pthread_self();
}
