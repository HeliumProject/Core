#include "PlatformPch.h"
#include "Platform/Trace.h"

#if HELIUM_ENABLE_TRACE

#include "Platform/Exception.h"
#include "Platform/MemoryHeap.h"

#include <cstdarg>

using namespace Helium;

Helium::Trace Helium::g_Trace;

/// Constructor.
Helium::Trace::Trace()
	: m_level( TraceLevels::Info )
	, m_lastMessageLevel( TraceLevels::Debug )
	, m_bNewLine( true )
{
}

/// Destructor.
Helium::Trace::~Trace()
{
}

/// Set the current logging level.
///
/// @param[in] level  Logging level.
///
/// @see GetLevel()
void Helium::Trace::SetLevel( TraceLevel level )
{
	MutexScopeLock scopeLock( m_mutex );

	m_level = level;
}

/// Write out a formatted message to this log.
///
/// @param[in] level    Logging level.
/// @param[in] pFormat  Format string.
/// @param[in] ...      Format arguments.
void Helium::Trace::Output( TraceLevel level, const char* pFormat, ... )
{
	va_list argList;
	va_start( argList, pFormat );
	OutputVa( level, pFormat, argList );
	va_end( argList );
}

/// Write out a formatted message to this log using a variable argument list.
///
/// @param[in] level    Logging level.
/// @param[in] pFormat  Format string.
/// @param[in] argList  Initialized variable argument list for the format arguments (va_start() should have already
///                     been called on this as necessary).
void Helium::Trace::OutputVa( TraceLevel level, const char* pFormat, va_list argList )
{
	HELIUM_ASSERT( pFormat );

	MutexScopeLock scopeLock( m_mutex );

	if( level < m_level )
	{
		return;
	}

	if( m_bNewLine || level != m_lastMessageLevel )
	{
		OutputImplementation( GetLevelString( level ) );
	}

	m_bNewLine = false;
	m_lastMessageLevel = level;

	char buffer[ DEFAULT_MESSAGE_BUFFER_SIZE ];

#if !HELIUM_OS_WIN
    va_list argListTemp;
	va_copy(argListTemp, argList);
#else
    va_list argListTemp = argList;
#endif
	int result = StringPrintArgs( buffer, HELIUM_ARRAY_COUNT( buffer ), pFormat, argListTemp );

	if( static_cast< unsigned int >( result ) < HELIUM_ARRAY_COUNT( buffer ) )
	{
		OutputImplementation( buffer );
		m_bNewLine = ( buffer[ result - 1 ] == TXT( '\n' ) );

		return;
	}

	if( result < 0 )
	{
#if !HELIUM_OS_WIN
		va_copy(argListTemp, argList);
#else
		argListTemp = argList;
#endif
		result = StringPrintArgs( NULL, 0, pFormat, argListTemp );
		HELIUM_ASSERT( result >= 0 );
	}

	size_t bufferSize = static_cast< size_t >( result ) + 1;

#if HELIUM_HEAP
	DefaultAllocator allocator;
	char* pBuffer = static_cast< char* >( allocator.Allocate( sizeof( char ) * bufferSize ) );
#else
	char* pBuffer = static_cast< char* >( ::malloc( sizeof( char ) * bufferSize ) );
#endif
	HELIUM_ASSERT( pBuffer );
	if( pBuffer )
	{
#if !HELIUM_OS_WIN
		va_copy(argListTemp, argList);
#else
		argListTemp = argList;
#endif
		result = StringPrintArgs( pBuffer, bufferSize, pFormat, argListTemp );

		HELIUM_ASSERT( result == static_cast< int >( bufferSize - 1 ) );
		OutputImplementation( pBuffer );
		m_bNewLine = ( pBuffer[ result - 1 ] == TXT( '\n' ) );

#if HELIUM_HEAP
		allocator.Free( pBuffer );
#else
		::free( pBuffer );
#endif
	}
}

/// Write out a message to this log.
///
/// @param[in] pMessage  Message text.
void Helium::Trace::OutputImplementation( const char* pMessage )
{
	HELIUM_ASSERT( pMessage );

#if !HELIUM_RELEASE && !HELIUM_PROFILE
	DebugLog( pMessage );
#endif
}

/// Get the string to display to indicate the specified logging level.
///
/// @param[in] level  Logging level.
///
/// @return  Logging level string.
const char* Helium::Trace::GetLevelString( TraceLevel level )
{
	switch( level )
	{
		case TraceLevels::Debug:
		{
			return TXT( "[D] " );
		}

		case TraceLevels::Info:
		{
			return TXT( "[I] " );
		}

		case TraceLevels::Warning:
		{
			return TXT( "[W] " );
		}

		case TraceLevels::Error:
		{
			return TXT( "[E] " );
		}
	}

	return TXT( "[?] " );
}

#endif  // HELIUM_ENABLE_TRACE
