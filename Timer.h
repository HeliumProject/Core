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
        static void StaticInitialize();
        static bool IsInitialized();

        static uint64_t GetTickCount();
        static float64_t GetSeconds();

        inline static uint64_t GetTicksPerSecond();
        inline static float64_t GetSecondsPerTick();
        //@}

    private:
        /// Tick count on static initialization.
        static uint64_t sm_startTickCount;
        /// Performance counter frequency.
        static uint64_t sm_ticksPerSecond;
        /// Seconds per performance counter tick.
        static float64_t sm_secondsPerTick;
    };
}

#include "Platform/Timer.inl"