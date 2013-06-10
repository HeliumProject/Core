#include "Platform/Assert.h"

#include <cstdio>

using namespace Helium;

/// Terminate the application on a fatal error.
///
/// @param[in] exitCode  Error code associated with the exit.
void Helium::FatalExit( int /*exitCode*/ )
{
    abort();
}

#if HELIUM_ASSERT_ENABLED

#if HELIUM_UNICODE
#define PRINTF wprintf
#else
#define PRINTF printf
#endif

/// Handle an assertion.
///
/// @param[in] pMessageText  Assert message text.
AssertResult Assert::TriggerImplementation( const char* pMessageText )
{
    PRINTF( "%s\n", pMessageText );

    return AssertResults::Break;
}

#endif  // HELIUM_ASSERT_ENABLED
