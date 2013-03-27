#include "PlatformPch.h"

#include "Platform/System.h"
#include "Platform/Timer.h"
#include "Platform/Assert.h"

using namespace Helium;

static bool s_IsInitialized = false;
uint64_t Timer::sm_ticksPerSecond = 0;
uint64_t Timer::sm_startTickCount = 0;
float64_t Timer::sm_secondsPerTick = 0.0;

/// Perform static initialization necessary to enable timing support.
void Timer::StaticInitialize()
{
    LARGE_INTEGER perfQuery;
    HELIUM_VERIFY( QueryPerformanceFrequency( &perfQuery ) );
    HELIUM_ASSERT( perfQuery.QuadPart != 0 );

    sm_ticksPerSecond = perfQuery.QuadPart;
    sm_secondsPerTick = 1.0 / static_cast< float64_t >( perfQuery.QuadPart );

    HELIUM_VERIFY( QueryPerformanceCounter( &perfQuery ) );
    sm_startTickCount = perfQuery.QuadPart;
    s_IsInitialized = true;
}

bool Timer::IsInitialized()
{
    return s_IsInitialized;
}

/// Get the current application timer tick count.
///
/// The timer tick frequency can vary depending on the platform and hardware on which the application is running.
/// To get the application tick frequency, use GetTicksPerSecond().
///
/// @return  Current application timer tick count.
///
/// @see GetTicksPerSecond(), GetSecondsPerTick(), GetSeconds()
uint64_t Timer::GetTickCount()
{
    LARGE_INTEGER perfCounter;
    QueryPerformanceCounter( &perfCounter );

    return perfCounter.QuadPart;
}

/// Get the number of seconds elapsed since StaticInitialize() was called.
///
/// @return  Elapsed seconds since static initialization.
///
/// @see GetSecondsPerTick(), GetTickCount(), GetTicksPerSecond()
float64_t Timer::GetSeconds()
{
    LARGE_INTEGER perfCounter;
    QueryPerformanceCounter( &perfCounter );

    return ( static_cast< float64_t >( perfCounter.QuadPart - sm_startTickCount ) * sm_secondsPerTick );
}

IntervalTimer::IntervalTimer()
    : m_Thread( GetCurrentThreadID() )
{
    m_Handle = ::CreateWaitableTimer(NULL, FALSE, NULL);
}

IntervalTimer::~IntervalTimer()
{
    ::CloseHandle( m_Handle );
}

void IntervalTimer::Set( int32_t timeoutInMs )
{
    LARGE_INTEGER dueTime;
    MemorySet( &dueTime, 0, sizeof( dueTime ) );
    dueTime.QuadPart = -(LONGLONG)( timeoutInMs * 10000.00 );
    ::SetWaitableTimer( m_Handle, &dueTime, 0, NULL, NULL, FALSE );
}

void IntervalTimer::Wait()
{
    // this limitation is to maintain POSIX signal timer-based functionality
    HELIUM_ASSERT( GetCurrentThreadID() == m_Thread );

    ::WaitForSingleObject( m_Handle, INFINITE );
}
