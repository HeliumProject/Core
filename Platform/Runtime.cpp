#include "Precompile.h"
#include "Runtime.h"

using namespace Helium;
using namespace Helium::Platform;

const char* Types::Strings[] = 
{
	"Unknown",
	"Windows",
	"Posix",
};

HELIUM_COMPILE_ASSERT( Types::Count == sizeof(Types::Strings) / sizeof(const char*) );

const char* Endiannesses::Strings[] = 
{
	"Little",
	"Big",
};

HELIUM_COMPILE_ASSERT( Endiannesses::Count == sizeof(Endiannesses::Strings) / sizeof(const char*) );

Platform::Endianness Platform::GetEndianness()
{
#if HELIUM_ENDIAN_LITTLE
    return Endiannesses::Little;
#else
    return Endiannesses::Big;
#endif
}
