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

#define HELIUM_MONGO_PROFILE 0

#if HELIUM_PROFILE_INSTRUMENT_ALL || HELIUM_MONGO_PROFILE
#define HELIUM_MONGO_SCOPE_TIMER(__Str) HELIUM_PROFILE_SCOPE_TIMER(__Str)
#else
#define HELIUM_MONGO_SCOPE_TIMER(__Str)
#endif
