#pragma once

#include "Platform/API.h"
#include "Platform/Assert.h"
#include "Platform/Locks.h"

/// @defgroup tracemacros Trace Macros
//@{

#if !defined( HELIUM_ENABLE_TRACE ) && !( HELIUM_RELEASE || HELIUM_PROFILE )
/// Defined and non-zero if tracing is enabled.
#define HELIUM_ENABLE_TRACE 1
#endif

#if defined( HELIUM_ENABLE_TRACE ) && !HELIUM_ENABLE_TRACE
#undef HELIUM_ENABLE_TRACE
#endif

#if HELIUM_ENABLE_TRACE

namespace Helium
{
	/// Trace levels.  These are intentionally declared outside the scope of the Trace class and abbreviated for ease of
	/// use.
	namespace TraceLevels
	{
		enum Type
		{
			Debug,    ///< Debug logging messages.
			Info,     ///< General info messages.
			Warning,  ///< Warning messages.
			Error,    ///< Critical error messages.
		};
	}
	typedef TraceLevels::Type TraceLevel;

	/// Trace interface.
	class HELIUM_PLATFORM_API Trace
	{
	public:
		/// Default size for formatted trace message buffers without requiring dynamic memory allocation.
		static const size_t DEFAULT_MESSAGE_BUFFER_SIZE = 1024;

		/// @name Logging Interface
		//@{
		static void SetLevel( TraceLevel level );
		static inline TraceLevel GetLevel();

		static void Output( TraceLevel level, const char* pFormat, ... );
		static void OutputVa( TraceLevel level, const char* pFormat, va_list argList );
		//@}

	protected:
		/// Current logging level.
		static TraceLevel sm_level;

		/// Logging level of the last message.
		static TraceLevel sm_lastMessageLevel;

		/// True if logging just started a fresh line.
		static bool sm_bNewLine;

		/// @name Logging Implementation
		//@{
		static void OutputImplementation( const char* pMessage );
		//@}

		/// @name Static Utility Functions
		//@{
		static const char* GetLevelString( TraceLevel level );
		//@}
	};
}

/// Write out a message to the trace output.
///
/// @param[in] LEVEL  Trace level.
/// @param[in] ...    Message.  This can be a "printf"-style format string and arguments or a Helium::String.
#define HELIUM_TRACE( LEVEL, ... ) { Helium::Trace::Output( LEVEL, __VA_ARGS__ ); HELIUM_ASSERT( LEVEL != Helium::TraceLevels::Error ); }

/// Write out a formatted message to the trace output using a variable argument list.
///
/// @param[in] LEVEL     Trace level.
/// @param[in] FORMAT    Format string.
/// @param[in] ARG_LIST  Variable argument list initialized to the format arguments (va_start() should have already been
///                      called.
#define HELIUM_TRACE_VA( LEVEL, FORMAT, ARG_LIST ) { Helium::Trace::OutputVa( LEVEL, FORMAT, ARG_LIST ); HELIUM_ASSERT( LEVEL != Helium::TraceLevels::Error ); }

/// Set the current trace level.
///
/// @param[in] LEVEL  Level to set.
///
/// @see HELIUM_TRACE_GET_LEVEL()
#define HELIUM_TRACE_SET_LEVEL( LEVEL ) Helium::Trace::SetLevel( LEVEL )

/// Get the current trace level.
///
/// @return  Current trace level.
///
/// @see HELIUM_TRACE_SET_LEVEL()
#define HELIUM_TRACE_GET_LEVEL() Helium::Trace::GetLevel()

#else  // HELIUM_ENABLE_TRACE

/// Write out a message to the trace output.
///
/// @param[in] LEVEL  Trace level.
/// @param[in] ...    Message.  This can be a "printf"-style format string and arguments or a Helium::String.
#define HELIUM_TRACE( LEVEL, ... )

/// Write out a formatted message to the trace output using a variable argument list.
///
/// @param[in] LEVEL     Trace level.
/// @param[in] FORMAT    Format string.
/// @param[in] ARG_LIST  Variable argument list initialized to the format arguments (va_start() should have already been
///                      called.
#define HELIUM_TRACE_VA( LEVEL, FORMAT, ARG_LIST )

/// Set the current trace level.
///
/// @param[in] LEVEL  Level to set.
///
/// @see HELIUM_TRACE_GET_LEVEL()
#define HELIUM_TRACE_SET_LEVEL( LEVEL )

/// Get the current trace level.
///
/// @return  Current trace level.
///
/// @see HELIUM_TRACE_SET_LEVEL()
#define HELIUM_TRACE_GET_LEVEL() ( Helium::Trace::LEVEL_INVALID )

#endif  // HELIUM_ENABLE_TRACE

//@}

#include "Platform/Trace.inl"
