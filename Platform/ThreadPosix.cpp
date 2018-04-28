#include "Precompile.h"
#include "Thread.h"

#include "Platform/Assert.h"
#include "Platform/Exception.h"
#include "Platform/MemoryHeap.h"

#include <unistd.h>
#include <sched.h>
#include <sys/types.h>

#if HELIUM_OS_LINUX
# include <sys/prctl.h>
#endif

#if HELIUM_OS_MAC
struct args {
    int joined;
    pthread_t td;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    void **res;
};

static void *waiter(void *ap)
{
    struct args *args = (struct args *)ap;
    pthread_join(args->td, args->res);
    pthread_mutex_lock(&args->mtx);
    pthread_mutex_unlock(&args->mtx);
    args->joined = 1;
    pthread_cond_signal(&args->cond);
    return 0;
}

int pthread_timedjoin_np(pthread_t td, void **res, struct timespec *ts)
{
    pthread_t tmp;
    int ret;
    struct args args;
    args.td = td;
    args.res = res;

    pthread_mutex_init(&args.mtx, 0);
    pthread_cond_init(&args.cond, 0);
    pthread_mutex_lock(&args.mtx);

    ret = pthread_create(&tmp, 0, waiter, &args);
    if (ret) return 1;

    do ret = pthread_cond_timedwait(&args.cond, &args.mtx, ts);
    while (!args.joined && ret != ETIMEDOUT);

    pthread_cancel(tmp);
    pthread_join(tmp, 0);

    pthread_cond_destroy(&args.cond);
    pthread_mutex_destroy(&args.mtx);

    return args.joined ? 0 : ret;
}
#endif

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

#if HELIUM_OS_LINUX
const pid_t Helium::InvalidThreadId = ~0;
#elif HELIUM_OS_MAC
const pthread_t Helium::InvalidThreadId = reinterpret_cast< pthread_t >( ~0 );
#endif

Thread::Thread()
    : m_Handle( 0 )
    , m_Valid( false )
{
    m_Name[0] = char('\0');
}

Thread::~Thread()
{
    Join();
}

bool Thread::Start( const char* pName, ThreadPriority priority )
{
    HELIUM_ASSERT( priority >= ThreadPriorities::Lowest && priority <= ThreadPriorities::Inherit );

    // Cache the name
    CopyString( m_Name, pName );

    pthread_attr_t attr;
    HELIUM_VERIFY( pthread_attr_init(&attr) == 0 );
    HELIUM_VERIFY( pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) == 0 );
    if ( priority != ThreadPriorities::Inherit )
    {
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
    }

    HELIUM_VERIFY( pthread_attr_destroy(&attr) == 0);

    return IsValid();
}

bool Thread::Join( uint32_t timeOutMilliseconds )
{
    bool result = false;

    if ( IsValid() )
    {
        if ( timeOutMilliseconds )
        {
            struct timespec spec;
            spec.tv_sec = timeOutMilliseconds / 1000;
            spec.tv_nsec = ( timeOutMilliseconds % 1000 ) * 1000000;
            result = pthread_timedjoin_np( m_Handle, NULL, &spec ) == 0;
        }
        else
        {
            result = pthread_join( m_Handle, NULL ) == 0;
        }
    }

    if ( result )
    {
        m_Valid = false;
    }

    return result;
}

bool Thread::TryJoin()
{
    bool result = false;

    if ( IsValid() )
    {
        // zero signal here won't really send a signal, just poll for thread running state
#if HELIUM_OS_LINUX
        result = pthread_tryjoin_np( m_Handle, NULL ) == 0;
#elif HELIUM_OS_MAC
        if ( pthread_kill( m_Handle, 0 ) == ESRCH )
        {
            result = pthread_join( m_Handle, NULL ) == 0;
        }
#endif
    }

    if ( result )
    {
        m_Valid = false;
    }

    return result;
}

bool Thread::IsValid() const
{
    return m_Valid;
}

void Thread::Sleep( uint32_t milliseconds )
{
    usleep( milliseconds * 1000 );
}

void Thread::Yield()
{
    sched_yield();
}

ThreadId Thread::GetCurrentId()
{
    return pthread_self();
}

static ThreadId g_MainThread = pthread_self();

ThreadId Thread::GetMainId()
{
    return g_MainThread;
}

void* Thread::ThreadCallback( void* pData )
{
    HELIUM_ASSERT( pData );

    Thread* pThread = static_cast< Thread* >( pData );

#if HELIUM_OS_LINUX
    prctl( PR_SET_NAME, reinterpret_cast< unsigned long >( pThread->m_Name ) );
#elif HELIUM_OS_MAC
    pthread_setname_np( pThread->m_Name );
#endif

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
