#include "Precompile.h"
#include "Assert.h"

#include <cstdio>

using namespace Helium;

/// Terminate the application on a fatal error.
///
/// @param[in] exitCode  Error code associated with the exit.
void Helium::FatalExit(int /*exitCode*/)
{
	abort();
}

#if HELIUM_ASSERT_ENABLED

/// Handle an assertion.
///
/// @param[in] pMessageText  Assert message text.
bool Assert::TriggerImplementation(const char* pMessageText)
{
	printf("%s\n", pMessageText);
	return true;
}

#endif  // HELIUM_ASSERT_ENABLED
