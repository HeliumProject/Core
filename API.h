#pragma once

#include "Platform/System.h"

#include "Foundation/Profile.h"

#if HELIUM_SHARED
# ifdef HELIUM_MONGO_EXPORTS
#  define HELIUM_MONGO_API HELIUM_API_EXPORT
# else
#  define HELIUM_MONGO_API HELIUM_API_IMPORT
# endif
#else
#define HELIUM_MONGO_API
#endif

//#define MONGO_PROFILE

#if defined(PROFILE_INSTRUMENT_ALL) || defined(MONGO_PROFILE)
#define MONGO_SCOPE_TIMER(__Str) PROFILE_SCOPE_TIMER(__Str)
#else
#define MONGO_SCOPE_TIMER(__Str)
#endif
