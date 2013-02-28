#include "Platform/Thread.h"
#include "Platform/Platform.h"

#include "Platform/Assert.h"

using namespace Helium;

Thread::Thread()
: m_Handle (0x0)
{

}

Thread::~Thread()
{
    if (Valid())
    {
        Close();
    }
}

bool Thread::Create(Entry entry, void* obj, const tchar* name, int priority)
{
    HELIUM_ASSERT( false );
    return true;
}

Thread::Return Thread::Exit()
{
    HELIUM_ASSERT( false );
}

void Thread::Close()
{
    m_Handle = 0x0;
}

Thread::Return Thread::Wait(u32 timeout)
{
    HELIUM_ASSERT( false );
}

bool Thread::Running()
{
    HELIUM_ASSERT( false );
}

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

void* ThreadLocalPointer::GetPointer()
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

u32 Helium::GetMainThreadID()
{
    HELIUM_BREAK();
    return 0;
}

u32 Helium::GetCurrentThreadID()
{
    HELIUM_BREAK();
    return 0;
}
