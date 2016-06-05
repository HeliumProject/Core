#pragma once

#if __clang_major__ < 3
#error Clang 3 required.
#endif

#define HELIUM_CC_CLANG 1
#define HELIUM_CC_CLANG_VERSION ( __clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__ )

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

/// Mark variable as actually used (omit unused variable warning)
#define HELIUM_UNUSED(expr) (void)(expr)
