#include "Precompile.h"
#include "Trace.h"

#if HELIUM_ENABLE_TRACE

#include "Platform/Exception.h"
#include "Platform/MemoryHeap.h"

#include <cstdarg>

using namespace Helium;

TraceLevel Trace::sm_level = TraceLevels::Info;
TraceLevel Trace::sm_lastMessageLevel = TraceLevels::Debug;
bool Trace::sm_bNewLine = true;

/// Set the current logging level.
///
/// @param[in] level  Logging level.
///
/// @see GetLevel()
void Trace::SetLevel( TraceLevel level )
{
	sm_level = level;
}

/// Write out a formatted message to this log.
///
/// @param[in] level    Logging level.
/// @param[in] pFormat  Format string.
/// @param[in] ...      Format arguments.
void Trace::Output( TraceLevel level, const char* pFormat, ... )
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
void Trace::OutputVa( TraceLevel level, const char* pFormat, va_list argList )
{
	HELIUM_ASSERT( pFormat );

	static uint8_t mutexStorage[ sizeof( Mutex ) ];
	static Mutex* pMutex = new ( mutexStorage ) Mutex;
	MutexScopeLock scopeLock( *pMutex );

	if( level < sm_level )
	{
		return;
	}

	if( sm_bNewLine || level != sm_lastMessageLevel )
	{
		OutputImplementation( GetLevelString( level ) );
	}

	sm_bNewLine = false;
	sm_lastMessageLevel = level;

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
		sm_bNewLine = ( buffer[ result - 1 ] == '\n' );

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
		sm_bNewLine = ( pBuffer[ result - 1 ] == '\n' );

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
void Trace::OutputImplementation( const char* pMessage )
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
const char* Trace::GetLevelString( TraceLevel level )
{
	switch( level )
	{
		case TraceLevels::Debug:
		{
			return "[D] ";
		}

		case TraceLevels::Info:
		{
			return "[I] ";
		}

		case TraceLevels::Warning:
		{
			return "[W] ";
		}

		case TraceLevels::Error:
		{
			return "[E] ";
		}
	}

	return "[?] ";
}

#endif  // HELIUM_ENABLE_TRACE
