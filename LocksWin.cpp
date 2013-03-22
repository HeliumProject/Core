#include "PlatformPch.h"
#include "Platform/Locks.h"
#include "Platform/Error.h"
#include "Platform/System.h"
#include "Platform/Assert.h"

using namespace Helium;

/// Constructor.
Mutex::Mutex()
{
    ::InitializeCriticalSection( &m_Handle );
}

/// Destructor.
Mutex::~Mutex()
{
    ::DeleteCriticalSection( &m_Handle );
}

/// Lock this mutex.
///
/// @see Unlock(), TryLock()
void Mutex::Lock()
{
    ::EnterCriticalSection( &m_Handle );
}

/// Unlock this mutex.
///
/// @see Lock(), TryLock()
void Mutex::Unlock()
{
    ::LeaveCriticalSection( &m_Handle );
}

/// Try to lock this mutex without blocking.
///
/// @return  True if the mutex was unlocked and this thread managed to acquire the lock, false the mutex is already
///          locked by another thread.
///
/// @see Lock(), Unlock()
bool Mutex::TryLock()
{
    return TryEnterCriticalSection( &m_Handle ) != FALSE;
}
