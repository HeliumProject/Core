#pragma once

#include <iostream>
#include <iomanip>
#include <string>

#include <vector>
#include <set>
#include <map>

#include "Platform/Types.h"

#include "Foundation/API.h"
#include "Foundation/Endian.h"

typedef uint64_t tuid;
#define TUID_HEX_FORMAT "0x%016" PRIX64
#define TUID_INT_FORMAT "%" PRIu64

namespace Helium
{
	class HELIUM_FOUNDATION_API TUID
	{
	protected:
		tuid m_ID;

	public:
		// The null ID
		static const tuid Null;

		// Generates a unique ID
		static tuid Generate();
		static void Generate( TUID& uid );
		static void Generate( tuid& uid );

		// Constructors
		inline TUID();
		inline TUID( tuid id );
		inline TUID( const TUID &id );
		inline TUID( const std::string& id );

		// Operators
		inline TUID& operator=( const TUID &rhs );
		inline bool operator==( const TUID &rhs ) const;
		inline bool operator==( const tuid &rhs ) const;
		inline bool operator!=( const TUID &rhs ) const;
		inline bool operator!=( const tuid &rhs ) const;
		inline bool operator<( const TUID &rhs ) const;

		// Interop with tuid
		inline operator tuid() const;

		// String conversion
		void ToString( std::string& id ) const;
		bool FromString( const std::string& str );
		void ToString( std::wstring& id ) const;
		bool FromString( const std::wstring& str );

		// Resets the ID to be the null ID
		inline void Reset();

		// Configure an iostream to output a TUID in the form 0x0000000000000000
		static std::ostream& HexFormat( std::ostream& base );
		static std::wostream& HexFormat( std::wostream& base );
	};

	template<>
	inline void Swizzle<TUID>(TUID& val, bool swizzle);
}

#include "Foundation/TUID.inl"
