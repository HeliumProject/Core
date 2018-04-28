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
				Unknown,
				Windows,
				Posix,
				Count,
			};

			HELIUM_PLATFORM_API extern const char* Strings[];
		}
		typedef Types::Type Type;

		HELIUM_PLATFORM_API Type GetType();

		inline const char* GetTypeString(Type t);

		namespace Endiannesses
		{
			enum Type
			{
				Little,
				Big,
				Count,
			};

			HELIUM_PLATFORM_API extern const char* Strings[];
		}
		typedef Endiannesses::Type Endianness;

		HELIUM_PLATFORM_API Endianness GetEndianness();

		inline const char* GetEndiannessString(Endianness e);
	}

	HELIUM_PLATFORM_API void EnableCPPErrorHandling( bool enable );
}

#include "Platform/Runtime.inl"