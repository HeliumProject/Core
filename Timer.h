#pragma once

#include "Platform/System.h"
#include "Platform/Utility.h"

#if HELIUM_OS_WIN
# include "Platform/TimerWin.h"
#endif

namespace Helium
{
    /// Application timer support.
    class HELIUM_PLATFORM_API Timer : public TimerPlatformData
    {
    public:
        /// @name Static Timing Support
        //@{
        static void StaticInitialize();
        static bool IsInitialized();

        static uint64_t GetTickCount();
        inline static uint64_t GetStartTickCount();
        inline static uint64_t GetTicksPerSecond();
        inline static float64_t GetSecondsPerTick();
        static float64_t GetSeconds();
        //@}
    };

    /// Interval-based timer (wait for a periodic timeout in real time)
    class HELIUM_PLATFORM_API IntervalTimer : NonCopyable
    {
	public:
#if HELIUM_OS_WIN
        typedef HANDLE Handle;
#endif

    public:
        IntervalTimer(bool manualReset = false);
        ~IntervalTimer();

        void Set( int32_t timeoutInMs );
        void Wait();

    private:
        Handle m_Handle;
    };
}

#if HELIUM_OS_WIN
# include "Platform/TimerWin.inl"
#endif
