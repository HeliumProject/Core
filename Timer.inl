/// Get the number of timer ticks per second.
///
/// @return  Timer tick frequency, in ticks per second.
///
/// @see GetTickCount(), GetSecondsPerTick(), GetSeconds()
uint64_t Helium::Timer::GetTicksPerSecond()
{
    return sm_ticksPerSecond;
}

/// Get the number of seconds per tick.
///
/// @return  Seconds per tick.
///
/// @see GetSeconds(), GetTickCount(), GetTicksPerSecond()
float64_t Helium::Timer::GetSecondsPerTick()
{
    return sm_secondsPerTick;
}