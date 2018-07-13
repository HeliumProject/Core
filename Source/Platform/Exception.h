#pragma once

#include "Platform/Error.h"
#include "Platform/Console.h"

#include <vector>
#include <exception>

namespace Helium
{
	//
	// Constants
	//

	const size_t ERROR_STRING_BUF_SIZE = 768; 

	//
	// Exception class
	//  Try to only throw in "error" cases. Examples:
	//   * A file format API trying to open a file that doesn't exist (the client api should check if it exists if it was graceful execution)
	//   * A disc drive or resource is out of space and cannot be written to
	//   * A network port is taken and cannot be bound
	//

	class HELIUM_PLATFORM_API Exception
	{
	protected:
		Exception();

	public:
		Exception( const char *msgFormat, ... );

		// These accessors are thow that re-throw blocks can amend the exception message
		inline std::string& Get();
		inline const std::string& Get() const;
		inline void Set(const std::string& message);

		// This allow operation with std::exception case statements
		virtual const char* What() const;

	protected:
		void SetMessage( const char* msgFormat, ... );
		void SetMessage( const char* msgFormat, va_list msgArgs );

		mutable std::string m_Message;
	};

	namespace ExceptionTypes
	{
		enum ExceptionType
		{
			CPP,
#if HELIUM_OS_WIN
			Structured,
#else
			Signal
#endif
		};

		static const char* Strings[] =
		{
			"C++",
#if HELIUM_OS_WIN
			"Structured",
#else
			"Signal",
#endif
		};
	}
	typedef ExceptionTypes::ExceptionType ExceptionType;

	struct HELIUM_PLATFORM_API ExceptionArgs
	{
		ExceptionType               m_Type;
		bool                        m_Fatal;
		std::string                 m_Message;
		std::string                 m_Callstack;
		std::vector< std::string >  m_Threads;
		std::string                 m_State;

		// CPP-specific info
		std::string                 m_CPPClass;

#if HELIUM_OS_WIN
		// SEH-specific info
		uint32_t                    m_SEHCode;
		std::string                 m_SEHClass;
		std::string                 m_SEHControlRegisters;
		std::string                 m_SEHIntegerRegisters;
#endif
		ExceptionArgs( ExceptionType type, bool fatal )
			: m_Type( type )
			, m_Fatal( fatal )
#if HELIUM_OS_WIN
			, m_SEHCode( -1 )
#endif
		{
		};
	};

	/// @defgroup debugutility Debug Utility Functions
	//@{
	// Detects if a debugger is attached to the process
	HELIUM_PLATFORM_API bool IsDebuggerPresent();

#if !HELIUM_RELEASE && !HELIUM_PROFILE
	HELIUM_PLATFORM_API bool InitializeSymbols( const std::string& path = "" );
	HELIUM_PLATFORM_API bool GetSymbolsInitialized();
	HELIUM_PLATFORM_API size_t GetStackTrace( void** ppStackTraceArray, size_t stackTraceArraySize, size_t skipCount = 1 );
	HELIUM_PLATFORM_API void GetAddressSymbol( std::string& rSymbol, void* pAddress );
	HELIUM_PLATFORM_API void DebugLog( const char* pMessage );
#endif
	//@}
}

#ifdef HELIUM_OS_WIN
# include "Platform/ExceptionWin.h"
#endif

#include "Platform/Exception.inl"