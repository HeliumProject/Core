#pragma once

#include "Platform/System.h"

#if HELIUM_SHARED
# ifdef HELIUM_PLATFORM_EXPORTS
#  define HELIUM_PLATFORM_API HELIUM_API_EXPORT
# else
#  define HELIUM_PLATFORM_API HELIUM_API_IMPORT
# endif
#else
# define HELIUM_PLATFORM_API
#endif
