#pragma once

#include <functional>

#include "Platform/Types.h"

#include "Foundation/API.h"

/// Comparison algorithm for strings sorted using case insensitive natural ordering.
HELIUM_FOUNDATION_API int strnatcmp(char const *a, char const *b);

/// Comparison algorithm for strings sorted using case insensitive natural ordering.
HELIUM_FOUNDATION_API int strinatcmp(char const *a, char const *b);

namespace Helium
{
	/// Comparison algorithm for strings sorted using case insensitive natural ordering.
	HELIUM_FOUNDATION_API inline int NaturalCompareString( char const* a, char const* b );

	/// Comparison algorithm for strings sorted using case insensitive natural ordering.
	HELIUM_FOUNDATION_API inline int CaseInsensitiveNaturalCompareString( char const* a, char const* b );

	/// Comparison algorithm for strings sorted using case insensitive natural ordering.
	struct HELIUM_FOUNDATION_API NaturalStringComparitor : public std::binary_function< std::string, std::string, bool >
	{
		inline bool operator()( const std::string& str1, const std::string& str2 ) const;
	};

	/// Comparison algorithm for strings sorted using case insensitive natural ordering.
	struct HELIUM_FOUNDATION_API CaseInsensitiveNaturalStringComparitor : public std::binary_function< std::string, std::string, bool >
	{
		inline bool operator()( const std::string& str1, const std::string& str2 ) const;
	};
}

#include "Foundation/Natural.inl"