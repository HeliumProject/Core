#include "PlatformPch.h"
#include "Platform/Profile.h"

using namespace Helium;

#pragma TODO("Move this class into Timer.h/cpp when Timer subsystems merge")

#if HELIUM_OS_WIN
const TraceFile::Handle TraceFile::InvalidHandleValue = NULL;
#else
const TraceFile::Handle TraceFile::InvalidHandleValue = -1;
#endif

void SimpleTimer::Reset()
{
    m_StartTime = Helium::TimerGetClock();
}

float SimpleTimer::Elapsed()
{
    return Helium::CyclesToMillis(Helium::TimerGetClock() - m_StartTime);
}
