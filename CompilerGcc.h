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

#if __cplusplus <= 199711L
// pull in type_traits to the global namespace for backward compatibility 
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
#endif
