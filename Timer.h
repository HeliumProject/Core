#pragma once

#include "Platform/System.h"
#include "Platform/Utility.h"

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
        static void StaticInitialize();
        static bool IsInitialized();

        static uint64_t GetTickCount();
        static float64_t GetSeconds();

        static uint64_t GetTicksPerSecond();
        static float64_t GetSecondsPerTick();
        //@}

    private:
#if HELIUM_OS_WIN
        /// Tick count on static initialization.
        static uint64_t sm_startTickCount;
        /// Performance counter frequency.
        static uint64_t sm_ticksPerSecond;
        /// Seconds per performance counter tick.
        static float64_t sm_secondsPerTick;
#endif
    };

    /// Interval-based timer (wait for a periodic timeout in real time)
    class HELIUM_PLATFORM_API IntervalTimer : NonCopyable
    {
    public:
        IntervalTimer(bool manualReset = false);
        ~IntervalTimer();

        void Set( int32_t timeoutInMs );
        void Wait();

    private:
#if HELIUM_OS_WIN
        typedef HANDLE Handle;
#else
        typedef timer_t Handle;
#endif
        Handle m_Handle;
    };
}
