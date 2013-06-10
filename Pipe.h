#pragma once

#include "Platform/Condition.h"

const static uint32_t IPC_PIPE_BUFFER_SIZE = 8192;

namespace Helium
{
	class HELIUM_PLATFORM_API Pipe
	{
	public:
#ifdef HELIUM_OS_WIN
		typedef void* Handle;
#else
        typedef int Handle;
#endif

        Pipe();
		~Pipe();

		bool Create(const char* name);
		bool Open(const char* name);
		void Close();

		bool Connect();
		void Disconnect();

		bool Read(void* buffer, uint32_t bytes, uint32_t& read);
		bool Write(void* buffer, uint32_t bytes, uint32_t& wrote);

	private:
        Handle m_Handle;

#if HELIUM_OS_WIN
		OVERLAPPED m_Overlapped;
        HANDLE     m_TerminateIo;
#endif
	};

	HELIUM_PLATFORM_API bool InitializePipes();
	HELIUM_PLATFORM_API void CleanupPipes();
}
