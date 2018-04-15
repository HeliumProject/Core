#include "Precompile.h"
#include "Locks.h"

#include "Platform/Atomic.h"

using namespace Helium;

/// Constructor.
ReadWriteLock::ReadWriteLock()
	: m_readLockCount( 0 )
	, m_readLockCountWithinWrite( 0 )
	, m_writeLockCountWithinWrite( 0 )
	, m_writeThread( InvalidThreadId )
	, m_readReleaseCondition( true, true )
	, m_writeReleaseCondition( true, true )
{
}

/// Destructor.
ReadWriteLock::~ReadWriteLock()
{
}

/// Acquire a non-exclusive, read-only lock for the current thread.
///
/// @see UnlockRead(), LockWrite(), UnlockWrite()
void ReadWriteLock::LockRead()
{
	// If we have the write lock and it's us, we can just bail. No need to track Lock/Unlocks
	if ( m_readLockCount == -1 && m_writeThread == Thread::GetCurrentId() )
	{
		++m_readLockCountWithinWrite;
		return;
	}

	int32_t currentLockCount = m_readLockCount;
	int32_t localLockCount;
	do 
	{
		localLockCount = currentLockCount;
		while( localLockCount == -1 )
		{
			while( !m_writeReleaseCondition.Wait() );
			localLockCount = m_readLockCount;
		}

		HELIUM_ASSERT( localLockCount != -2 );
		currentLockCount = AtomicCompareExchangeAcquire( m_readLockCount, localLockCount + 1, localLockCount );
	}
	while( currentLockCount != localLockCount );

	if( localLockCount == 0 )
	{
		m_readReleaseCondition.Reset();
	}
}

/// Release a read-only lock previously acquired using LockRead().
///
/// @see LockRead(), LockWrite(), UnlockWrite()
void ReadWriteLock::UnlockRead()
{
	// If we have the write lock and it's us, we can just bail. No need to track Lock/Unlocks
	if ( m_readLockCount == -1 && m_writeThread == Thread::GetCurrentId() )
	{
		--m_readLockCountWithinWrite;
		return;
	}

	HELIUM_ASSERT( m_readLockCount != -1 );

	int32_t newLockCount = AtomicDecrementRelease( m_readLockCount );
	if( newLockCount == 0 )
	{
		m_readReleaseCondition.Signal();
	}
}

/// Acquire an exclusive, read-write lock for the current thread.
///
/// @see UnlockWrite(), LockRead(), UnlockRead()
void ReadWriteLock::LockWrite()
{
	if ( m_writeThread == Thread::GetCurrentId() )
	{
		++m_writeLockCountWithinWrite;
		return;
	}

	for (;;)
	{
		int32_t currentLockCount = AtomicCompareExchangeAcquire( m_readLockCount, -1, 0 );
		if( currentLockCount == 0 )
		{
			break;
		}

		while( !m_readReleaseCondition.Wait() );
		while( !m_writeReleaseCondition.Wait() );
	}

	m_writeThread = Thread::GetCurrentId();
	m_writeReleaseCondition.Reset();
}

/// Release a read-write lock previously acquired using LockWrite().
///
/// @see LockWrite(), LockRead(), UnlockRead()
void ReadWriteLock::UnlockWrite()
{
	HELIUM_ASSERT( m_writeThread == Thread::GetCurrentId() );

	if (m_writeLockCountWithinWrite)
	{
		--m_writeLockCountWithinWrite;
		return;
	}

	HELIUM_ASSERT( m_readLockCountWithinWrite == 0 );
	m_writeThread = InvalidThreadId;

	HELIUM_ASSERT( m_readLockCount == -1 );
	AtomicExchangeRelease( m_readLockCount, 0 );
	m_writeReleaseCondition.Signal();
}
