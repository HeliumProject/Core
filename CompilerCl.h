#pragma once

#if _MSC_VER < 1900
#pragma message "Visual Studio 2015 is the minimum supported version."
#endif

#include <intrin.h>

#define HELIUM_CC_CL 1

/// DLL export API declaration.
#define HELIUM_API_EXPORT __declspec( dllexport )
/// DLL import API declaration.
#define HELIUM_API_IMPORT __declspec( dllimport )

/// Attribute for forcing the compiler to inline a function.
#define HELIUM_FORCEINLINE __forceinline

/// Attribute for explicitly defining a pointer or reference as not being externally aliased.
#define HELIUM_RESTRICT __restrict

/// Prefix macro for declaring type or variable alignment.
///
/// @param[in] ALIGNMENT  Byte alignment (must be a power of two).
#define HELIUM_ALIGN_PRE( ALIGNMENT ) __declspec( align( ALIGNMENT ) )

/// Suffix macro for declaring type or variable alignment.
///
/// @param[in] ALIGNMENT  Byte alignment (must be a power of two).
#define HELIUM_ALIGN_POST( ALIGNMENT )

/// Mark variable as actually used (omit unused variable warning)
#define HELIUM_UNUSED(expr) (void)(expr)

// Template classes shouldn't be DLL exported, but the compiler warns us by default.
#pragma warning( disable : 4251 ) // 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
// Visual C++ does not support exception specifications at this time, but we still want to retain them for compilers
// that do support them.  This is harmless to ignore.
#pragma warning( disable : 4290 ) // C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
// This spuriously comes up on occasion with certain template class methods.
#pragma warning( disable : 4505 ) // 'function' : unreferenced local function has been removed
