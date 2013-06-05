#include "PlatformPch.h"
#include "Platform/Timer.h"

#include "Platform/System.h"
#include "Platform/Assert.h"
#include "Platform/Thread.h"
#include "Platform/Locks.h"

#include <unistd.h>
#include <sys/times.h>

using namespace Helium;

static bool s_IsInitialized = false;
uint64_t Timer::sm_ticksPerSecond = 0;
uint64_t Timer::sm_startTickCount = 0;
float64_t Timer::sm_secondsPerTick = 0.0;

/// Perform static initialization necessary to enable timing support.
void Timer::StaticInitialize()
{
    uint64_t ticksPerSecond = sysconf(_SC_CLK_TCK);
    HELIUM_ASSERT( ticksPerSecond != 0 );
    sm_ticksPerSecond = ticksPerSecond;
    sm_secondsPerTick = 1.0 / static_cast< float64_t >( ticksPerSecond );

    struct tms timing = { 0 };
    clock_t ticks = times( &timing );
    HELIUM_VERIFY( ticks );
    sm_startTickCount = ticks;

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
    struct tms timing = { 0 };
    return times( &timing );
}

/// Get the number of seconds elapsed since StaticInitialize() was called.
///
/// @return  Elapsed seconds since static initialization.
///
/// @see GetSecondsPerTick(), GetTickCount(), GetTicksPerSecond()
float64_t Timer::GetSeconds()
{
    struct tms timing = { 0 };
    clock_t ticks = times( &timing );
    return ( static_cast< float64_t >( ticks - sm_startTickCount ) * sm_secondsPerTick );
}

#if !HELIUM_OS_MAC

IntervalTimer::IntervalTimer()
: m_Thread( GetCurrentThreadID() )
, m_Signal( 0 )
, m_WakeupsMissed( 0 )
{
    int sig;

    static Mutex signalMutex;
    {
        ScopeLock< Mutex > lock ( signalMutex );
        static int next_sig;
        if (next_sig == 0)
        {
            next_sig = SIGRTMIN;
        }

        // Check that we have not run out of signals
        HELIUM_ASSERT(next_sig <= SIGRTMAX);
        sig = next_sig;
        next_sig++;
    }

    m_Signal = sig;

    // Create the signal mask that will be used in wait_period
    sigemptyset( &m_AlarmSet );
    sigaddset( &m_AlarmSet, m_Signal );

    // Create a timer that will generate the signal we have chosen
    struct sigevent sigev;
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = m_Signal;
    sigev.sigev_value.sival_ptr = (void*)&m_Handle;
    HELIUM_VERIFY( timer_create (CLOCK_MONOTONIC, &sigev, &m_Handle) == 0 );
}

IntervalTimer::~IntervalTimer()
{
    timer_delete( m_Handle );
}

void IntervalTimer::Set( int32_t timeoutInMs )
{
    unsigned int sec = timeoutInMs / 1000;
    unsigned int ns = ( timeoutInMs % 1000 ) * 1000000;

    struct itimerspec itval;
    itval.it_interval.tv_sec = sec;
    itval.it_interval.tv_nsec = ns;
    itval.it_value.tv_sec = sec;
    itval.it_value.tv_nsec = ns;
    HELIUM_VERIFY( timer_settime (m_Handle, 0, &itval, NULL) == 0 );
}

void IntervalTimer::Wait()
{
    HELIUM_ASSERT( GetCurrentThreadID() == m_Thread );

    int sig;
    sigwait( &m_AlarmSet, &sig );
    m_WakeupsMissed += timer_getoverrun( m_Handle );
}

#endif
