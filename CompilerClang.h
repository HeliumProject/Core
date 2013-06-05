#pragma once

/// Declare a class method as overriding a virtual method of a parent class.
#define HELIUM_OVERRIDE
/// Declare a class as an abstract base class.
#define HELIUM_ABSTRACT

/// DLL export API declaration.
#define HELIUM_API_EXPORT
/// DLL import API declaration.
#define HELIUM_API_IMPORT

/// Attribute for forcing the compiler to inline a function.
#define HELIUM_FORCEINLINE inline __attribute__( ( always_inline ) )

/// Attribute for explicitly defining a pointer or reference as not being externally aliased.
#define HELIUM_RESTRICT __restrict

/// Prefix macro for declaring type or variable alignment.
///
/// @param[in] ALIGNMENT  Byte alignment (must be a power of two).
#define HELIUM_ALIGN_PRE( ALIGNMENT )

/// Suffix macro for declaring type or variable alignment.
///
/// @param[in] ALIGNMENT  Byte alignment (must be a power of two).
#define HELIUM_ALIGN_POST( ALIGNMENT ) __attribute__( ( aligned( ALIGNMENT ) ) )

#include <type_traits>
namespace std
{
	template< class T > struct has_trivial_assign : is_trivially_assignable< T, T > {};
	template< class T > struct has_trivial_constructor : is_trivially_constructible< T > {};
	template< class T > struct has_trivial_destructor : is_trivially_destructible< T > {};
	template< class T > struct has_trivial_copy : is_trivially_copyable< T > {};
}