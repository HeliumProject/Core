#include "Precompile.h"
#include "Runtime.h"

using namespace Helium;
using namespace Helium::Platform;

Platform::Type Platform::GetType()
{
    return Types::Posix;
}

void Helium::EnableCPPErrorHandling( bool enable )
{
}
