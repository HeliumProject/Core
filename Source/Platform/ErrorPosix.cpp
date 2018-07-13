#include "Precompile.h"
#include "Error.h"

#include <errno.h>

using namespace Helium;

uint32_t Helium::GetLastError()
{
    return errno;
}

std::string Helium::GetErrorString( uint32_t errorOverride )
{
    return "Unknown";
}
