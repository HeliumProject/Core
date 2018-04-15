#include "Precompile.h"
#include "Semaphore.h"

#include "Platform/Assert.h"

using namespace Helium;

Semaphore::Semaphore()
{
#if HELIUM_OS_MAC
    char addr[19];
    sprintf( addr, "0x%016lX", reinterpret_cast< intptr_t >( this ) );
    int result = (m_Handle = sem_open(addr, O_CREAT, 0644, 0)) == SEM_FAILED ? 1 : 0;
#else
    int result = sem_init(&m_Handle, 0, 0);
#endif
    HELIUM_ASSERT( result == 0 );
}

Semaphore::~Semaphore()
{
#if HELIUM_OS_MAC
    int result = sem_close( m_Handle );
    if ( result == 0 )
    {
        char addr[19];
        sprintf( addr, "0x%016lX", reinterpret_cast< intptr_t >( this ) );
        result = sem_unlink( addr );
    }
#else
    int result = sem_destroy(&m_Handle);
#endif
    HELIUM_ASSERT( result == 0 );
}

void Semaphore::Increment()
{
#if HELIUM_OS_MAC
    sem_t* s = m_Handle;
#else
    sem_t* s = &m_Handle;
#endif
    int result = sem_post(s);
    HELIUM_ASSERT( result == 0 );
}

void Semaphore::Decrement()
{
#if HELIUM_OS_MAC
    sem_t* s = m_Handle;
#else
    sem_t* s = &m_Handle;
#endif
    int result = sem_wait(s);
    HELIUM_ASSERT( result == 0 );
}

void Semaphore::Reset()
{
    this->~Semaphore();
    new (this) Semaphore();
}
