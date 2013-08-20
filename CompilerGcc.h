#pragma once

#if __cplusplus < 201103L
# define HELIUM_CPP11 0
#else
# define HELIUM_CPP11 1
#endif

/// Declare a class method as overriding a virtual method of a parent class.
#define HELIUM_OVERRIDE
/// Declare a class as an abstract base class.
#define HELIUM_ABSTRACT

/// DLL export API declaration.
#define HELIUM_API_EXPORT
/// DLL import API declaration.
#define HELIUM_API_IMPORT

/// Attribute for forcing the compiler to inline a function.
#if __GNUC__ < 4
#define HELIUM_FORCEINLINE inline  // GCC 3 support for "always_inline" is somewhat bugged, so fall back to just "inline".
#else
#define HELIUM_FORCEINLINE inline __attribute__( ( always_inline ) )
#endif

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

#define HELIUM_GCC_VERSION ( __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ )

#if HELIUM_CPP11
# include <type_traits>
#endif

#if HELIUM_GCC_VERSION >= 40800

namespace std
{
	// untested, but might work (its what works in LLVM Clang)
	template< class T > struct has_trivial_assign : is_trivially_assignable< T, T > {};
	template< class T > struct has_trivial_constructor : is_trivially_constructible< T > {};
	template< class T > struct has_trivial_destructor : is_trivially_destructible< T > {};
	template< class T > struct has_trivial_copy : is_trivially_copyable< T > {};
}

#else // HELIUM_GCC_VERSION >= 40800

#if HELIUM_CPP11

namespace std
{
	template< class T > struct has_trivial_assign : has_trivial_copy_assign< T > {};
	template< class T > struct has_trivial_constructor : has_trivial_default_constructor< T > {};
	template< class T > struct has_trivial_copy : has_trivial_copy_assign< T > {};
}

#else // HELIUM_CPP11

#include <tr1/type_traits>

namespace std
{
	namespace tr1 {}
	using tr1::true_type;
	using tr1::false_type;
	using tr1::integral_constant;
	using tr1::is_base_of;
	using tr1::is_pointer;
	using tr1::is_signed;
	using tr1::is_array;
	using tr1::remove_cv;
	using tr1::remove_extent;
	using tr1::alignment_of;
	using tr1::has_trivial_assign;
	using tr1::has_trivial_constructor;
	using tr1::has_trivial_destructor;
	using tr1::has_trivial_copy;
}

#endif // HELIUM_CPP11

#endif // HELIUM_GCC_VERSION >= 40800