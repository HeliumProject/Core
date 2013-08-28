#pragma once

#if __GNUC__ < 4
#error GCC 4 required.
#endif

#define HELIUM_CC_GCC_VERSION ( __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ )

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
