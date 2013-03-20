#include "Platform/Profile.h"
#include "Platform/Assert.h"

#include <pthread.h>
#include <assert.h>

void Helium::TraceFile::Open(const tchar_t* file)
{

}

void Helium::TraceFile::Close()
{

}

void Helium::TraceFile::Write(const tchar_t* data, int size)
{

}

const tchar_t* Helium::TraceFile::GetFilePath()
{
    return NULL;
}

uint64_t Helium::TimerGetClock()
{
    uint64_t time = 0;
    HELIUM_BREAK();
    return time;
}

float Helium::CyclesToMillis(uint64_t cycles)
{
    return (float64_t)cycles * (float64_t)(1000.0 / 79800000ULL);
}

float Helium::TimeTaken(uint64_t start_time)
{
    uint64_t time = TimerGetClock() - start_time;
    return CyclesToMillis(time);
}

void Helium::ReportTime(const tchar_t* segment, uint64_t start_time, double& total_millis)
{
    uint64_t time = TimerGetClock() - start_time;
    double millis = CyclesToMillis(time);
    printf("%s took %f ms\n", segment, millis);
    total_millis += millis;
}

uint64_t Helium::GetTotalMemory()
{
    return 256 * (1 << 20);
}
