#include "Locks.h"

#include "Platform/Assert.h"

#include <pthread.h>

using namespace Helium;

Mutex::Mutex()
{
    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m_Handle, &mta);
    pthread_mutexattr_destroy(&mta);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_Handle);
}

void Mutex::Lock()
{
    int result = pthread_mutex_lock(&m_Handle);
    HELIUM_ASSERT( result == 0 );
}

void Mutex::Unlock()
{
    int result = pthread_mutex_unlock(&m_Handle);
    HELIUM_ASSERT( result == 0 );
}

bool Mutex::TryLock()
{
	int result = pthread_mutex_trylock(&m_Handle);
    return result == 0;
}
