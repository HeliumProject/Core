#include "PlatformPch.h"
#include "Platform/Timer.h"

using namespace Helium;

float64_t Timer::TicksToMilliseconds( uint64_t ticks )
{
	return ( static_cast<float64_t>(ticks) * 1000.0 ) * GetTicksPerSecond();
}

SimpleTimer::SimpleTimer()
{
	Reset();
}

void SimpleTimer::Reset()
{
    m_StartTime = Timer::GetTickCount();
}

float64_t SimpleTimer::Elapsed()
{
	return Timer::TicksToMilliseconds( Timer::GetTickCount() - m_StartTime );
}
