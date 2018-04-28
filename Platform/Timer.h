#pragma once

#include "Platform/System.h"
#include "Platform/Utility.h"
#include "Platform/Thread.h"

#if !HELIUM_OS_WIN
# include <signal.h>
# include <time.h>
#endif

namespace Helium
{
	/// Application timer support.
	class HELIUM_PLATFORM_API Timer
	{
	public:
		/// @name Static Timing Support
		//@{
		static uint64_t GetTickCount();
		static uint64_t GetTicksPerSecond();
		static float64_t GetSecondsPerTick();
		static float64_t TicksToMilliseconds( uint64_t ticks );
		//@}

	private:
		/// Performance counter frequency.
		static uint64_t sm_ticksPerSecond;
		/// Seconds per performance counter tick.
		static float64_t sm_secondsPerTick;
	};

	/// Simple timer object.
	class HELIUM_PLATFORM_API SimpleTimer : NonCopyable
	{
	public:
		/// Initialize the timer (start timing).
		SimpleTimer();

		/// Restart the timer.
		void Reset();

		/// Get elapsed time in milliseconds.
		float64_t Elapsed();

	private:
		/// Tick count since the last time the timer was reset.
		uint64_t m_StartTime;
	};
}
