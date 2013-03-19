#include "Platform/Error.h"

using namespace Helium;

uint32_t Helium::GetLastError()
{
    return 0xffffffff;
}

tstring Helium::GetErrorString( uint32_t errorOverride )
{
    return "Unknown";
}
