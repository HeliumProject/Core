#include "Platform/Error.h"

#include <errno.h>

using namespace Helium;

uint32_t Helium::GetLastError()
{
    return errno;
}

tstring Helium::GetErrorString( uint32_t errorOverride )
{
    return "Unknown";
}
