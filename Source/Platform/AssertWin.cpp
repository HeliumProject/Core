#include "Precompile.h"
#include "Assert.h"

#include "Platform/SystemWin.h"
#include "Platform/Console.h"
#include "Platform/Encoding.h"

#if HELIUM_OS_WIN

using namespace Helium;

/// Terminate the application on a fatal error.
///
/// @param[in] exitCode  Error code associated with the exit.
void Helium::FatalExit( int exitCode )
{
	::FatalExit( exitCode );
}

#if HELIUM_ASSERT_ENABLED

/// Handle an assertion.
///
/// @param[in] pMessageText  Assert message text.
bool Assert::TriggerImplementation( const char* pMessageText )
{
	Helium::FatalExit(-1);
	if ( ::IsDebuggerPresent() )
	{
		return true;
	}

	char messageBoxText[ 1024 ];
	StringPrint(
		messageBoxText,
		( "%s\n\nChoose \"Abort\" to terminate the program, \"Retry\" to debug the program (if a debugger "
		  "is attached), or \"Ignore\" to attempt to skip over the error." ),
		pMessageText );

	HELIUM_TCHAR_TO_WIDE( messageBoxText, wideMessageBoxText );
	switch( MessageBoxW( NULL, wideMessageBoxText, L"Assert", MB_ABORTRETRYIGNORE ) )
	{
	case IDABORT:
		Helium::FatalExit( -1 );

	case IDIGNORE:
		return false;

	default:
		return true;
	}
}

#endif  // HELIUM_OS_WIN

#endif  // HELIUM_ASSERT_ENABLED
