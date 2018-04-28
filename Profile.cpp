#include "Precompile.h"
#include "Profile.h"

#include "Platform/Assert.h"
#include "Platform/Thread.h"
#include "Platform/System.h"
#include "Platform/Types.h"

#include "Foundation/Log.h"
#include "Foundation/String.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef MIN
#define MIN(A,B)        ((A) < (B) ? (A) : (B))
#endif
#ifndef MAX
#define MAX(A,B)        ((A) > (B) ? (A) : (B))
#endif

using namespace Helium;
using namespace Helium::Profile;

static uint32_t  g_SinkCount = 0;
static Sink*     g_Sinks[HELIUM_PROFILE_SINK_MAX];
static uint32_t  g_ContextCount = 0;
static Context*  g_Contexts[HELIUM_PROFILE_CONTEXTS_MAX];
static bool      g_Enabled = false;

void Profile::Startup()
{
	g_Enabled = true;
}

void Profile::Shutdown()
{
	for ( uint32_t i = 0; i < g_ContextCount; ++i )
	{
		g_Contexts[i]->FlushFile();
		delete( g_Contexts[i] );
	}
	g_ContextCount = 0;
	g_Enabled = false;
}

Sink::Sink(const char* name)
	: m_Function(NULL)
	, m_File(NULL)
	, m_Line(0)
	, m_Hits(0)
	, m_Millis(0.0f)
	, m_Index(-1)
{
	CopyString(m_Name, name);

	Init();
}

Sink::Sink(const char* func, const char* file, uint32_t line)
	: m_Function(func)
	, m_File(file)
	, m_Line(line)
	, m_Hits(0)
	, m_Millis(0.0f)
	, m_Index(-1)
{
	StringPrint(m_Name, "%s() %s:%d", func, file, line);

	Init();
}

void Sink::Init()
{
	HELIUM_ASSERT(m_Name[0] != '\0');

	if ( m_Index < 0 && g_SinkCount < HELIUM_PROFILE_SINK_MAX )
	{
		g_Sinks[g_SinkCount] = this;
		m_Index = g_SinkCount++;
	}
}

Sink::~Sink()
{
	if ( m_Index >= 0 )
	{
		g_Sinks[m_Index] = NULL;
	}
}

void Sink::Report()
{
	Log::Profile("[%12.3f] [%8d] %s\n", m_Millis, m_Hits, m_Name);
}

int CompareLocationPtr(const void* ptr1, const void* ptr2)
{
	const Sink* left = *(const Sink**)ptr1;
	const Sink* right = *(const Sink**)ptr2;

	if ( left && !right )
	{
		return -1;
	}

	if ( !left && right )
	{
		return 1;
	}

	if ( left && right )
	{
		if ( ( left )->m_Millis > ( right )->m_Millis )
		{
			return -1;
		}
		else if ( ( left )->m_Millis < ( right )->m_Millis )
		{
			return 1;
		}
	}

	return 0;
}

void Sink::ReportAll()
{
	float totalTime = 0.f;
	for ( uint32_t i = 0; i < g_SinkCount; i++ )
	{
		if ( g_Sinks[i] )
		{
			totalTime += g_Sinks[i]->m_Millis;
		}
	}

	if ( totalTime > 0.f )
	{
		Log::Profile("\nProfile Report:\n");

		qsort(g_Sinks, g_SinkCount, sizeof(Sink*), &CompareLocationPtr);

		for ( uint32_t i = 0; i < g_SinkCount; i++ )
		{
			if ( g_Sinks[i] && g_Sinks[i]->m_Millis > 0.f )
			{
				g_Sinks[i]->Report();
			}
		}
	}
}

Helium::ThreadLocalPointer g_ProfileContext;

Profile::Timer::Timer(Sink& sink, const char* fmt, ...)
	: m_Sink(sink)
{
	if ( fmt )
	{
		va_list args;
		va_start(args, fmt);
		StringPrintArgs(m_Name, fmt, args);
		va_end(args);
	}
	else
	{
		m_Name[0] = '\0';
	}

	m_StartTicks = Helium::Timer::GetTickCount();

#if HELIUM_PROFILE_INSTRUMENTATION

	Context* context = (Context*)g_ProfileContext.GetPointer();

	if ( context == NULL )
	{
		context = new Context;
		g_ProfileContext.SetPointer(context);

		// save it off. this should probably be locked
		g_Contexts[g_ContextCount] = context;
		g_ContextCount++;

		InitPacket* init = context->AllocPacket<InitPacket>(HELIUM_PROFILE_CMD_INIT);

		init->m_Version = HELIUM_PROFILE_PROTOCOL_VERSION;
		init->m_Signature = HELIUM_PROFILE_SIGNATURE;
		init->m_Conversion = static_cast<float32_t>( Helium::Timer::TicksToMilliseconds(HELIUM_PROFILE_CYCLES_FOR_CONVERSION) );
	}

	ScopeEnterPacket* enter = context->AllocPacket<ScopeEnterPacket>(HELIUM_PROFILE_CMD_SCOPE_ENTER);

	enter->m_UniqueID = context->m_UniqueID++;
	enter->m_StackDepth = context->m_StackDepth;
	enter->m_Line = m_Sink.m_Line;
	enter->m_StartTicks = m_StartTicks;

	CopyString(enter->m_Description, m_Name);

	if ( m_Sink.m_Function )
	{
		CopyString(enter->m_Function, m_Sink.m_Function);
	}
	else
	{
		enter->m_Function[0] = '\0';
	}

	context->m_StackDepth++;
	if ( m_Sink.m_Index != -1 )
	{
		context->m_SinkStack[m_Sink.m_Index]++;
	}

#endif
}

Profile::Timer::~Timer()
{
	uint64_t stopTicks = Helium::Timer::GetTickCount();

	uint64_t   taken = stopTicks - m_StartTicks;
	float millis = static_cast<float32_t>( Helium::Timer::TicksToMilliseconds(taken) );

	if ( m_Name[0] != '\0' )
	{
		Log::Profile("[%12.3f] %s\n", millis, m_Name);
	}

#if HELIUM_PROFILE_INSTRUMENTATION

	Context* context = (Context*)g_ProfileContext.GetPointer();
	HELIUM_ASSERT(context);

	ScopeExitPacket* packet = context->AllocPacket<ScopeExitPacket>(HELIUM_PROFILE_CMD_SCOPE_EXIT);

	packet->m_UniqueID = context->m_UniqueID++;
	packet->m_StackDepth = --context->m_StackDepth;
	packet->m_Duration = taken;

	if ( m_Sink.m_Index != -1 )
	{
		int stack = --context->m_SinkStack[m_Sink.m_Index];

		if ( stack == 0 )
		{
			m_Sink.m_Millis += millis;
		}

		m_Sink.m_Hits++;
	}

#else

	if ( m_Sink.m_Index != -1 )
	{
		m_Sink.m_Millis += millis;
		m_Sink.m_Hits++;
	}

#endif
}

Context::Context()
	: m_UniqueID(0)
	, m_StackDepth(0)
	, m_PacketBufferOffset(0)
{
	m_TraceFile.Open("profile.bin", FileModes::Write);
	memset(m_SinkStack, 0, sizeof(m_SinkStack));
}

Context::~Context()
{
	m_TraceFile.Close();
}

void Context::FlushFile()
{
	uint64_t startTicks = Helium::Timer::GetTickCount();

	// make a scope enter packet for flushing the file
	ScopeEnterPacket* enter = (ScopeEnterPacket*)( m_PacketBuffer + m_PacketBufferOffset );
	m_PacketBufferOffset += sizeof(ScopeEnterPacket);

	enter->m_Header.m_Command = HELIUM_PROFILE_CMD_SCOPE_ENTER;
	enter->m_Header.m_Size = sizeof(ScopeEnterPacket);
	enter->m_UniqueID = 0;
	enter->m_StackDepth = 0;
	enter->m_Line = __LINE__;
	enter->m_StartTicks = startTicks;
	strcpy(enter->m_Function, "Context::FlushFile");
	enter->m_Description[0] = 0;

	// make a block end packet for end of packet
	BlockEndPacket* blockEnd = (BlockEndPacket*)( m_PacketBuffer + m_PacketBufferOffset );
	m_PacketBufferOffset += sizeof(BlockEndPacket);

	blockEnd->m_Header.m_Command = HELIUM_PROFILE_CMD_BLOCK_END;
	blockEnd->m_Header.m_Size = sizeof(BlockEndPacket);

	// we write the whole buffer, in large blocks
	m_TraceFile.Write((const char*)m_PacketBuffer, HELIUM_PROFILE_PACKET_BLOCK_SIZE);

	// reset the packet buffer
	m_PacketBufferOffset = 0;

	// make a scope exit packet for being done flushing the file
	ScopeExitPacket* exit = (ScopeExitPacket*)( m_PacketBuffer + m_PacketBufferOffset );
	m_PacketBufferOffset += sizeof(ScopeExitPacket);

	exit->m_Header.m_Command = HELIUM_PROFILE_CMD_SCOPE_EXIT;
	exit->m_Header.m_Size = sizeof(ScopeExitPacket);

	exit->m_UniqueID = 0;
	exit->m_StackDepth = 0;
	exit->m_Duration = Helium::Timer::GetTickCount() - startTicks;

	// return to filling out the packet buffer
}
