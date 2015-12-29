#pragma once 

#include "Platform/Types.h"
#include "Platform/File.h"
#include "Platform/Timer.h"

#include "Foundation/API.h"

#define HELIUM_PROFILE_STRINGIFY(x) #x
#define HELIUM_PROFILE_TOSTRING(x) HELIUM_PROFILE_STRINGIFY(x)

#define HELIUM_PROFILE_STRING_MAX              (256)
#define HELIUM_PROFILE_SINK_MAX                (2048)
#define HELIUM_PROFILE_CONTEXTS_MAX            (128)

#define HELIUM_PROFILE_PROTOCOL_VERSION        (0x00)
#define HELIUM_PROFILE_SIGNATURE               (0x12345678)

#define HELIUM_PROFILE_CMD_INIT                (0x00)
#define HELIUM_PROFILE_CMD_SCOPE_ENTER         (0x01)
#define HELIUM_PROFILE_CMD_SCOPE_EXIT          (0x02)
#define HELIUM_PROFILE_CMD_BLOCK_END           (0x03)

#define HELIUM_PROFILE_PACKET_STRING_BUFSIZE   (64)
#define HELIUM_PROFILE_CYCLES_FOR_CONVERSION   (100000)
#define HELIUM_PROFILE_PACKET_BLOCK_SIZE       (16 * 1024)

//
// Profile code API, for the most part almost all of this code always gets compiled in
// In general its beneficial to leave the accumulation API on all the time.
//

namespace Helium
{
	namespace Profile
	{
		HELIUM_FOUNDATION_API void Startup();
		HELIUM_FOUNDATION_API void Shutdown();

		class HELIUM_FOUNDATION_API Sink
		{
		public:
			Sink(const char* name);
			Sink(const char* func, const char* file, uint32_t line);
			~Sink();

			void Init();
			void Report();
			static void ReportAll();

			char        m_Name[HELIUM_PROFILE_STRING_MAX];
			const char* m_Function;
			const char* m_File;
			uint32_t    m_Line;
			uint32_t    m_Hits;
			float       m_Millis;
			int32_t     m_Index;
		};

		class HELIUM_FOUNDATION_API Timer
		{
		public:
			Timer(Sink& sink, const char* fmt = NULL, ...);
			~Timer();

			char     m_Name[HELIUM_PROFILE_STRING_MAX];
			Sink&    m_Sink;
			uint64_t m_StartTicks;
			uint32_t m_UniqueID;

		private:
			Timer(const Timer& rhs); // no implementation
		};

		struct Header
		{
			uint16_t m_Command;
			uint16_t m_Size;
		};

		struct InitPacket
		{
			Header    m_Header;
			uint32_t  m_Version;
			uint32_t  m_Signature;
			float32_t m_Conversion; // PROFILE_CYCLES_FOR_CONVERSION cycles -> how many millis?
		};

		struct ScopeEnterPacket
		{
			Header   m_Header;
			uint32_t m_UniqueID;
			uint32_t m_StackDepth;
			uint32_t m_Line;
			uint64_t m_StartTicks;
			char     m_Description[HELIUM_PROFILE_PACKET_STRING_BUFSIZE];
			char     m_Function[HELIUM_PROFILE_PACKET_STRING_BUFSIZE];
		};

		struct ScopeExitPacket
		{
			Header   m_Header;
			uint32_t m_UniqueID;
			uint32_t m_StackDepth;
			uint64_t m_Duration;
		};

		struct BlockEndPacket
		{
			Header m_Header;
		};

		union UberPacket
		{
			Header           m_Header;
			InitPacket       m_Init;
			ScopeEnterPacket m_ScopeEnter;
			ScopeExitPacket  m_ScopeExitPacket;
		};

		class HELIUM_FOUNDATION_API Context
		{
		public:
			File     m_TraceFile;
			uint32_t m_UniqueID;
			uint32_t m_StackDepth;
			uint32_t m_PacketBufferOffset;
			uint8_t  m_PacketBuffer[HELIUM_PROFILE_PACKET_BLOCK_SIZE];
			uint32_t m_SinkStack[HELIUM_PROFILE_SINK_MAX];

			Context();
			~Context();

			void FlushFile();

			template <class T>
			T* AllocPacket(uint32_t cmd)
			{
				uint32_t spaceNeeded = sizeof(T) + sizeof(BlockEndPacket) + sizeof(ScopeEnterPacket);

				if ( m_PacketBufferOffset + spaceNeeded >= HELIUM_PROFILE_PACKET_BLOCK_SIZE )
				{
					FlushFile();
				}

				T* packet = (T*)( m_PacketBuffer + m_PacketBufferOffset );
				m_PacketBufferOffset += sizeof(T);

				//Log::Print("CMD %d OFFSET %d\n", cmd, m_PacketBufferOffset);

				packet->m_Header.m_Command = cmd;
				packet->m_Header.m_Size = sizeof(T);

				return packet;
			}
		};
	}
}

// master profile enable
#if HELIUM_PROFILE
# define HELIUM_PROFILE_ENABLE 1
#else
# define HELIUM_PROFILE_ENABLE 0
#endif

//
// Instrumentation API pervades more code blocks and provides fine-grain profile data to a log file
//

// instrumentation api enable
#if HELIUM_PROFILE_ENABLE
# define HELIUM_PROFILE_INSTRUMENTATION 1
#else
# define HELIUM_PROFILE_INSTRUMENTATION 0
#endif

// flag to instrument all code possible
#if HELIUM_PROFILE_INSTRUMENTATION
# define HELIUM_PROFILE_INSTRUMENT_ALL 1
#else
# define HELIUM_PROFILE_INSTRUMENT_ALL 0
#endif

// instrumentation macros
#if HELIUM_PROFILE_INSTRUMENTATION

# define HELIUM_PROFILE_FUNCTION_TIMER() \
	static Helium::Profile::Sink functionSink ( HELIUM_FUNCTION_NAME, __FILE__, __LINE__ ); \
	Helium::Profile::Timer functionTimer ( functionSink );

# define HELIUM_PROFILE_SCOPE_TIMER( ... ) \
	static Helium::Profile::Sink scopeSink ( HELIUM_FUNCTION_NAME, __FILE__, __LINE__ ); \
	Helium::Profile::Timer scopeTimer ( scopeSink, __VA_ARGS__ );

#else

# define HELIUM_PROFILE_FUNCTION_TIMER()
# define HELIUM_PROFILE_SCOPE_TIMER( ... )

#endif
