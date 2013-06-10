#pragma once

#include "Platform/API.h"
#include "Platform/Types.h"
#include "Platform/Assert.h"

namespace Helium
{
    namespace Platform
    {
        namespace Types
        {
            enum Type
            {
                Windows,
                Posix,
                Count,
            };

            static const char* Strings[] = 
            {
                TXT("Windows"),
                TXT("Posix"),
            };

            HELIUM_COMPILE_ASSERT( Platform::Types::Count == sizeof(Strings) / sizeof(const char*) );
        }
        typedef Types::Type Type;

        inline const char* GetTypeName(Type t)
        {
            if (t >= 0 && t<Types::Count)
            {
                return Types::Strings[t];
            }
            else
            {
                return Types::Strings[0];
            }
        }

        HELIUM_PLATFORM_API Type GetType();
    }

    HELIUM_PLATFORM_API void EnableCPPErrorHandling( bool enable );
}
