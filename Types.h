#pragma once

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <stdint.h>
#include <stddef.h>

#include "Platform/System.h"

//
// Register types
//

#if HELIUM_CC_CL

/// @defgroup intprintf Integer "printf" Formatting Macros
/// These allow portable usage of fixed-sized integers in formatting strings for printf() and similar statements.
/// Macros defined in inttypes.h are used when available on supported platforms.
//@{

/// "char" string format macro for signed 8-bit integers.
#define PRId8 "hhd"
/// "char" string format macro for signed 16-bit integers.
#define PRId16 "hd"
/// "char" string format macro for signed 32-bit integers.
#define PRId32 "I32d"
/// "char" string format macro for signed 64-bit integers.
#define PRId64 "I64d"

/// "char" string format macro for unsigned 8-bit integers.
#define PRIu8 "hhu"
/// "char" string format macro for unsigned 16-bit integers.
#define PRIu16 "hu"
/// "char" string format macro for unsigned 32-bit integers.
#define PRIu32 "I32u"
/// "char" string format macro for unsigned 64-bit integers.
#define PRIu64 "I64u"

/// "char" string format macro for int_fast8_t.
#define PRIdFAST8 PRId8
/// "char" string format macro for int_fast16_t.
#define PRIdFAST16 PRId32
/// "char" string format macro for int_fast32_t.
#define PRIdFAST32 PRId32
/// "char" string format macro for int_fast64_t.
#define PRIdFAST64 PRId64

/// "char" string format macro for uint_fast8_t.
#define PRIuFAST8 PRIu8
/// "char" string format macro for uint_fast16_t.
#define PRIuFAST16 PRIu32
/// "char" string format macro for uint_fast32_t.
#define PRIuFAST32 PRIu32
/// "char" string format macro for uint_fast64_t.
#define PRIuFAST64 PRIu64

/// "char" string format macro for ptrdiff_t.
#define PRIdPD "Id"
/// "char" string format macro for size_t.
#define PRIuSZ "Iu"

//@}

/// @defgroup intscanf Integer "scanf" Formatting Macros
/// These allow portable usage of fixed-sized integers in formatting strings for scanf() and similar statements.  Macros
/// defined in inttypes.h are used instead on supported platforms.
//@{

/// "char" string format macro for signed 8-bit integers.
#define SCNd8 "hhd"
/// "char" string format macro for signed 16-bit integers.
#define SCNd16 "hd"
/// "char" string format macro for signed 32-bit integers.
#define SCNd32 "I32d"
/// "char" string format macro for signed 64-bit integers.
#define SCNd64 "I64d"

/// "char" string format macro for unsigned 8-bit integers.
#define SCNu8 "hhu"
/// "char" string format macro for unsigned 16-bit integers.
#define SCNu16 "hu"
/// "char" string format macro for unsigned 32-bit integers.
#define SCNu32 "I32u"
/// "char" string format macro for unsigned 64-bit integers.
#define SCNu64 "I64u"

/// "char" string format macro for int_fast8_t.
#define SCNdFAST8 SCNd8
/// "char" string format macro for int_fast16_t.
#define SCNdFAST16 SCNd32
/// "char" string format macro for int_fast32_t.
#define SCNdFAST32 SCNd32
/// "char" string format macro for int_fast64_t.
#define SCNdFAST64 SCNd64

/// "char" string format macro for uint_fast8_t.
#define SCNuFAST8 SCNu8
/// "char" string format macro for uint_fast16_t.
#define SCNuFAST16 SCNu32
/// "char" string format macro for uint_fast32_t.
#define SCNuFAST32 SCNu32
/// "char" string format macro for uint_fast64_t.
#define SCNuFAST64 SCNu64

/// "char" string format macro for ptrdiff_t.
#define SCNdPD "Id"
/// "char" string format macro for size_t.
#define SCNuSZ "Iu"

//@}

/// @defgroup stringprintf Char/String "printf" Formatting Macros
/// These allow portable usage of fixed-sized characters in formatting strings for printf() and similar statements.
//@{

#define PRItc "c"
#define PRIts "s"
#define PRIc "hc"
#define PRIs "hs"
#define PRIlc "lc"
#define PRIls "ls"

//@}

#else  // HELIUM_CC_CL

// Use inttypes.h where available; we simply try to provide relevant type definitions for platforms that don't provide it.
#include <inttypes.h>

#if defined(__GNUC__)
/*these two are compiler defined */
/// "char" string format macro for ptrdiff_t.
#define PRIdPD TXT( "td" )
// "char" string format macro for size_t.
#define PRIuSZ TXT( "zu" )
#endif

#endif  // HELIUM_CC_CL

/// @defgroup floattypes Floating-point Types
/// While these may not be particularly necessary, they do provide some level of consistency with the integer types.
//@{

/// Single-precision floating-point.
typedef float float32_t;
/// Double-precision floating-point.
typedef double float64_t;

//@}

//
// String types
//

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

/// Prefix for declaring string and character literals of the default character type.
#define TXT( X ) X
