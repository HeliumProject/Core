#include "Platform/Thread.h"
//#include "Platform/Platform.h"

#include "Platform/Assert.h"

using namespace Helium;

Thread::Thread( const tchar_t* pName )
: m_Handle( 0 )
, m_Name( NULL )
{
	SetName( pName );
}

Thread::~Thread()
{
    if (this->Valid())
    {
        this->Join();
    }
}

inline void setSchedParam(struct sched_param * params, ThreadPriority priority)
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
bool Thread::Start( ThreadPriority pri )
{
	HELIUM_ASSERT( pri >= ThreadPriorities::Lowest && pri <= ThreadPriorities::Highest );
    pthread_attr_t attr;
    struct sched_param sched;
    setSchedParam(&sched, pri);

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

/* I'm not sure this should be here, i think it's handled by the thread class now. -jsarrett */
/*
bool Thread::Create(Entry entry, void* obj, const tchar* name, int priority)
{
    HELIUM_ASSERT( false );
    return true;
}

Thread::Return Thread::Exit()
{
    int ok = pthread_cancel(&(this->m_Handle));
    HELIUM_ASSERT( ok );
}

Thread::Return Thread::Wait(u32 timeout)
{
    HELIUM_ASSERT( false );
}

bool Thread::Running()
{
    HELIUM_ASSERT( false );
}
*/

bool Thread::Valid()
{
    return m_Handle != 0;
}

ThreadLocalPointer::ThreadLocalPointer()
{
#if defined(PS3_POSIX) || defined(HELIUM_OS_LINUX)
    int status = pthread_key_create(&m_Key, NULL);
    HELIUM_ASSERT( status == 0 && "Could not create pthread_key");
#endif

    SetPointer(NULL);

    HELIUM_BREAK();
}

ThreadLocalPointer::~ThreadLocalPointer()
{
#if defined(PS3_POSIX) || defined(HELIUM_OS_LINUX)
    pthread_key_delete(m_Key);
#endif

    HELIUM_BREAK();
}

void* ThreadLocalPointer::GetPointer() const
{
#if defined(PS3_POSIX) || defined(HELIUM_OS_LINUX)
    return pthread_getspecific(m_Key);
#endif

    HELIUM_BREAK();
    return NULL;
}

void ThreadLocalPointer::SetPointer(void* pointer)
{
#if defined(PS3_POSIX) || defined(HELIUM_OS_LINUX)
    pthread_setspecific(m_Key, pointer);
#endif

    HELIUM_BREAK();
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
