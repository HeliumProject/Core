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

    /// Interval-based timer (wait for a periodic timeout in real time)
    ///  Note: this will only kick 1 thread each time it expires
    ///  If you want multiple threads to kick then wait them on a Condition and signal
    ///  it in a thread waiting on this timer.
    class HELIUM_PLATFORM_API IntervalTimer : NonCopyable
    {
    public:
        IntervalTimer();
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

#if !HELIUM_OS_WIN
        Thread::id_t m_Thread;
        sigset_t     m_AlarmSet;
        int          m_Signal;
        int          m_WakeupsMissed;
#endif
    };
}

#include "Platform/Timer.inl"