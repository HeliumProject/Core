#pragma once

#include "Platform/API.h"
#include "Platform/Types.h"

namespace Helium
{
    HELIUM_PLATFORM_API uint32_t GetLastError();
    HELIUM_PLATFORM_API std::string GetErrorString( uint32_t errorOverride = 0 );
}