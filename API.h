#pragma once

#include "Platform/System.h"

#include "Foundation/Profile.h"

#if HELIUM_SHARED
# ifdef HELIUM_PERSIST_EXPORTS
#  define HELIUM_PERSIST_API HELIUM_API_EXPORT
# else
#  define HELIUM_PERSIST_API HELIUM_API_IMPORT
# endif
#else
#define HELIUM_PERSIST_API
#endif

//#define PERSIST_PROFILE

#if defined(PROFILE_INSTRUMENT_ALL) || defined(PERSIST_PROFILE)
#define PERSIST_SCOPE_TIMER(__Str) PROFILE_SCOPE_TIMER(__Str)
#else
#define PERSIST_SCOPE_TIMER(__Str)
#endif
