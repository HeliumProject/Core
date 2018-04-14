#include "PlatformPch.h"
#include "Platform/Assert.h"
#include "Platform/Trace.h"
#include "Platform/Console.h"
#include "Platform/Exception.h"
#include <vector>

#if HELIUM_ASSERT_ENABLED

#include "Platform/Atomic.h"

using namespace Helium;

volatile int32_t Assert::sm_active = 0;

/// Handle an assertion.
///
/// @param[in] pExpression  String containing the expression that failed, or null if the assertion was
///                         unconditional.
/// @param[in] pMessage     Message associated with the assertion, or null if no customized message was given.
/// @param[in] pFunction    Function in which the assertion occurred.
/// @param[in] pFile        File in which the assertion occurred.
/// @param[in] line         Line number at which the assertion occurred.
bool Assert::Trigger(
	const char* pExpression,
	const char* pFunction,
	const char* pFile,
	int line,
	const char* pMessage,
	... )
{
	// Only allow one assert handler to be active at a time.
	while( AtomicCompareExchangeAcquire( sm_active, 1, 0 ) != 0 )
	{
	}

	char messageText[ 1024 ];

	if( pExpression )
	{
		if( pMessage )
		{
			va_list args;
			va_start(args, pMessage); 
			char message[1024];
			StringPrintArgs(message, pMessage, args);
			va_end(args); 

			StringPrint(
				messageText,
				"%s\n\nAssertion failed in %s (%s, line %d): (%s)",
				message,
				pFunction,
				pFile,
				line,
				pExpression );
		}
		else
		{
			StringPrint(
				messageText,
				"Assertion failed in %s (%s, line %d): %s",
				pFunction,
				pFile,
				line,
				pExpression );
		}
	}
	else
	{
		if( pMessage )
		{
			va_list args;
			va_start(args, pMessage); 
			char message[1024];
			StringPrintArgs(message, pMessage, args);
			va_end(args); 

			StringPrint(
				messageText,
				"%s\n\nAssertion failed in %s (%s, line %d)",
				message,
				pFunction,
				pFile,
				line );
		}
		else
		{
			StringPrint(
				messageText,
				"Assertion failed in %s (%s, line %d)",
				pFunction,
				pFile,
				line );
		}
	}

#if HELIUM_ENABLE_TRACE
	Helium::Trace::Output( TraceLevels::Error, "%s\n", messageText );
# if HELIUM_OS_WIN
#  if !HELIUM_RELEASE && !HELIUM_PROFILE
    if (Helium::GetSymbolsInitialized())
    {
        std::vector<uintptr_t> trace;
        Helium::GetStackTrace( trace, 1 );
        std::string str = "Stack Trace:\n";
        Helium::TranslateStackTrace( trace, str );
        Helium::Trace::Output( TraceLevels::Error, str.c_str() );
    }
    else
    {
        std::string str = "Stack trace unavailable - symbols not loaded\n";
        Helium::Trace::Output( TraceLevels::Error, str.c_str() );
    }
#  else
    std::string str = "Stack trace unavailable - symbols not loaded\n";
    HELIUM_TRACE( TraceLevels::Error, str.c_str() );
#  endif
# endif
#endif

	// Present the assert message and get how we should proceed.
	bool result = TriggerImplementation( messageText );

	AtomicExchangeRelease( sm_active, 0 );

	return result;
}

#endif  // HELIUM_ASSERT_ENABLED
