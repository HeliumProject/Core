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
