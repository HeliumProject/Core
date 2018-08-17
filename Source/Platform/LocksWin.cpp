#include "Precompile.h"
#include "Locks.h"

#include "Platform/SystemWin.h"
#include "Platform/Error.h"
#include "Platform/Assert.h"

using namespace Helium;

HELIUM_COMPILE_ASSERT( sizeof( Mutex::Handle::Debug ) == sizeof( RTL_CRITICAL_SECTION_DEBUG ) );
HELIUM_COMPILE_ASSERT( sizeof( Mutex::Handle ) == sizeof( CRITICAL_SECTION ) );

/// Constructor.
Mutex::Mutex()
{
    ::InitializeCriticalSection( reinterpret_cast<CRITICAL_SECTION*>( &m_Handle ) );
}

/// Destructor.
Mutex::~Mutex()
{
    ::DeleteCriticalSection( reinterpret_cast<CRITICAL_SECTION*>(&m_Handle) );
}

/// Lock this mutex.
///
/// @see Unlock(), TryLock()
void Mutex::Lock()
{
    ::EnterCriticalSection( reinterpret_cast<CRITICAL_SECTION*>(&m_Handle) );
}

/// Unlock this mutex.
///
/// @see Lock(), TryLock()
void Mutex::Unlock()
{
    ::LeaveCriticalSection( reinterpret_cast<CRITICAL_SECTION*>(&m_Handle) );
}

/// Try to lock this mutex without blocking.
///
/// @return  True if the mutex was unlocked and this thread managed to acquire the lock, false the mutex is already
///          locked by another thread.
///
/// @see Lock(), Unlock()
bool Mutex::TryLock()
{
    return TryEnterCriticalSection( reinterpret_cast<CRITICAL_SECTION*>(&m_Handle) ) != FALSE;
}
