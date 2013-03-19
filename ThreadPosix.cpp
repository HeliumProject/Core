#include "Platform/Thread.h"

#include "Platform/Assert.h"

using namespace Helium;

static void SetSchedParam(struct sched_param * params, ThreadPriority priority)
{
    switch (priority)
    {
    case ThreadPriority::Highest:
        params->sched_priority = 99;
        break;
    case ThreadPriority::High:
        params->sched_priority = 75;
        break;
    case ThreadPriority::Normal:
    default:
        params->sched_priority = 50;
        break;
    case ThreadPriority::Low:
        params->sched_priority = 25;
        break;
    case ThreadPriority::Lowest:
        params->sched_priority = 1;
        break;
    }
}

Thread::Thread( const tchar_t* pName )
: m_Handle( 0 )
, m_Name( NULL )
{
    SetName( pName );
}

Thread::~Thread()
{
    if ( this->Valid() )
    {
        this->Join();
    }
}

bool Thread::Start( ThreadPriority pri )
{
    HELIUM_ASSERT( pri >= ThreadPriorities::Lowest && pri <= ThreadPriorities::Highest );
    pthread_attr_t attr;
    struct sched_param sched;
    SetSchedParam(&sched, pri);

    HELIUM_ASSERT( pthread_attr_init(&attr) == 0);
    HELIUM_ASSERT( pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED) == 0 );
    HELIUM_ASSERT( pthread_attr_setschedparam(&attr, &sched) == 0 );

    int result = pthread_create( &(this->m_Handle), &attr, ThreadCallback, this);

    HELIUM_ASSERT( pthread_attr_destroy(&attr) == 0);
    bool ok = (result == VALID_ID);

    if (!ok)
    {
        this->m_Handle = 0;
    }

    return ok;
}

bool Thread::Valid()
{
    return m_Handle != 0;
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

uint32_t Helium::GetMainThreadID()
{
    HELIUM_BREAK();
    return 0;
}

uint32_t Helium::GetCurrentThreadID()
{
    HELIUM_BREAK();
    return 0;
}
