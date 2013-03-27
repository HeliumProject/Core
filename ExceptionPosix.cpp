#include "PlatformPch.h"
#include "Exception.h"

#include "Platform/Types.h"
#include "Platform/Trace.h"
#include "Platform/Console.h"
#include "Platform/Process.h"

#if !HELIUM_RELEASE && !HELIUM_PROFILE

/// Write a string to any platform-specific debug log output.
///
/// @param[in] pMessage  Message string to write to the debug log.
void Helium::DebugLog( const tchar_t* pMessage )
{
	Print( ConsoleColors::Red, stderr, pMessage );
}

#endif  // !HELIUM_RELEASE && !HELIUM_PROFILE
