#include "Precompile.h"
#include "Timer.h"

using namespace Helium;

static bool s_IsInitialized = false;
uint64_t Timer::sm_ticksPerSecond = 0;
float64_t Timer::sm_secondsPerTick = 0.0;

float64_t Timer::TicksToMilliseconds( uint64_t ticks )
{
	return ( static_cast<float64_t>( ticks ) * 1000.0 ) / static_cast<float64_t>( GetTicksPerSecond() );
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
