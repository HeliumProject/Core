#include "Precompile.h"
#include "Timer.h"

#include "Platform/SystemWin.h"
#include "Platform/Assert.h"

using namespace Helium;

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

/// Get the number of timer ticks per second.
///
/// @return  Timer tick frequency, in ticks per second.
///
/// @see GetTickCount(), GetSecondsPerTick(), GetSeconds()
uint64_t Timer::GetTicksPerSecond()
{
	if ( sm_ticksPerSecond == 0 )
	{
		LARGE_INTEGER perfQuery;
		HELIUM_VERIFY( QueryPerformanceFrequency( &perfQuery ) );
		HELIUM_ASSERT( perfQuery.QuadPart != 0 );
		sm_ticksPerSecond = perfQuery.QuadPart;
	}

	return sm_ticksPerSecond;
}

/// Get the number of seconds per tick.
///
/// @return  Seconds per tick.
///
/// @see GetSeconds(), GetTickCount(), GetTicksPerSecond()
float64_t Timer::GetSecondsPerTick()
{
	if ( sm_secondsPerTick == 0 )
	{
		LARGE_INTEGER perfQuery;
		HELIUM_VERIFY( QueryPerformanceFrequency( &perfQuery ) );
		HELIUM_ASSERT( perfQuery.QuadPart != 0 );
		sm_secondsPerTick = 1.0 / static_cast< float64_t >( perfQuery.QuadPart );
	}

	return sm_secondsPerTick;
}
