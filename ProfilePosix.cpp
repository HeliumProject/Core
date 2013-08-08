#include "Platform/Profile.h"
#include "Platform/Assert.h"

#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

void Helium::TraceFile::Open(const char* file)
{

}

void Helium::TraceFile::Close()
{

}

void Helium::TraceFile::Write(const char* data, int size)
{

}

const char* Helium::TraceFile::GetFilePath()
{
    return NULL;
}

#if HELIUM_CC_GCC || HELIUM_CC_CLANG

#if defined(__i386__)

static HELIUM_FORCEINLINE uint64_t rdtsc(void)
{
    uint64_t int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}
#elif defined(__x86_64__)

static HELIUM_FORCEINLINE uint64_t rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

#endif

#endif

uint64_t Helium::TimerGetClock()
{
    uint64_t time = rdtsc();
    return time;
}

inline double GetClockSpeed()
{
    static double clockSpeed = 0.0;
    if (clockSpeed != 0.0)
    {
        return clockSpeed;
    }

#if HELIUM_OS_LINUX
    int h = open( "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", O_RDONLY );
    if ( HELIUM_VERIFY( h >= 0 ) )
    {
        char buf[ 256 ];
        size_t bytesRead = read( h, buf, sizeof( buf ) );
        sscanf( buf, "%lf", &clockSpeed );
        clockSpeed *= 1000.f;
        close( h );
    }
#else
    HELIUM_BREAK();
#endif

    return clockSpeed;
}

float Helium::CyclesToMillis(uint64_t cycles)
{
    return (float64_t)cycles * (float64_t)(1000.0 / GetClockSpeed());
}

float Helium::TimeTaken(uint64_t start_time)
{
    uint64_t time = TimerGetClock() - start_time;
    return CyclesToMillis(time);
}

void Helium::ReportTime(const char* segment, uint64_t start_time, double& total_millis)
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
