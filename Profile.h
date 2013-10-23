#pragma once 

#include "Platform/API.h"
#include "Platform/MemoryHeap.h"
#include "Platform/Types.h"
#include "Platform/Utility.h"

namespace Helium
{
	class HELIUM_PLATFORM_API SimpleTimer : NonCopyable
	{
	public:
		SimpleTimer()
		{
			Reset();
		}

		// reset timer (for re-use)
		void Reset();

		// get elapsed time in millis
		float Elapsed();

	private:
		uint64_t m_StartTime;
	};

	class HELIUM_PLATFORM_API TraceFile
	{
	private:
#ifdef HELIUM_OS_WIN
		typedef FILE* Handle;
#else
		typedef int Handle;
#endif
		const static Handle InvalidHandleValue;

		Handle m_FileHandle;

	public:
		TraceFile()
			: m_FileHandle (InvalidHandleValue)
		{

		}

		void Open(const char* file);
		void Close();
		void Write(const char* data, int size);

		static const char* GetFilePath();
	};

	HELIUM_PLATFORM_API uint64_t TimerGetClock();
	HELIUM_PLATFORM_API float CyclesToMillis(uint64_t cycles);
	HELIUM_PLATFORM_API float TimeTaken(uint64_t start_time);
	HELIUM_PLATFORM_API void ReportTime(const char* segment, uint64_t start_time, double& total_millis);

	HELIUM_PLATFORM_API uint64_t GetTotalMemory();
}
