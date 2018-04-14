#pragma once

// NULL, offsetof, etc...
#include <stddef.h>

// std sizing typedefs
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>

// std printf macros
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

#include "Platform/System.h"

#if HELIUM_CC_CL

/// "char" string format macro for ptrdiff_t.
#define PRIdPD "Id"
/// "char" string format macro for size_t.
#define PRIuSZ "Iu"

#elif HELIUM_CC_GCC || HELIUM_CC_CLANG

/// "char" string format macro for ptrdiff_t.
# define PRIdPD "td"
/// "char" string format macro for size_t.
# define PRIuSZ "zu"

#else

#error Unknown compiler for defining format macros!

#endif

/// @defgroup floattypes Floating-point Types
/// While these may not be particularly necessary, they do provide some level of consistency with the integer types.
//@{

/// Single-precision floating-point.
typedef float float32_t;
/// Double-precision floating-point.
typedef double float64_t;

//@}

#if HELIUM_CPU_X86_32
# define HELIUM_PRINT_POINTER "08" PRIXPTR
#elif HELIUM_CPU_X86_64
# define HELIUM_PRINT_POINTER "016" PRIXPTR
#else
#error Unknown register width!
#endif

// TODO: remove these once stl usage is expunged
#include <string>
#include <fstream>
#include <sstream>

#ifdef _UNICODE
# ifndef UNICODE
#  define UNICODE
# endif
#endif

#ifdef UNICODE
# ifndef _UNICODE
#  define _UNICODE
# endif
#endif
