#pragma once

// Operating system (HELIUM_OS_*) macros:
// - HELIUM_OS_PC: PC operating system (any)
//   - HELIUM_OS_WIN: Windows (any architecture/version)
//     - HELIUM_OS_WIN32: 32-bit Windows
//     - HELIUM_OS_WIN64: 64-bit Windows

// CPU architecture (HELIUM_CPU_*) macros:
// - HELIUM_CPU_X86: Intel x86-based (any architecture)
//   - HELIUM_CPU_X86_32: 32-bit Intel x86
//   - HELIUM_CPU_X86_64: 64-bit Intel x86

// Compiler (HELIUM_CC_*) macros:
// - HELIUM_CC_CL: Microsoft Visual C++
// - HELIUM_CC_GCC: GCC

#if defined( _WIN64 )
# define HELIUM_OS_WIN 1
# define HELIUM_OS_WIN64 1
# define HELIUM_CPU_X86 1
# define HELIUM_CPU_X86_64 1
#elif defined( _WIN32 )
# define HELIUM_OS_WIN 1
# define HELIUM_OS_WIN32 1
# define HELIUM_CPU_X86 1
# define HELIUM_CPU_X86_32 1
#elif defined( __APPLE__ )
# define HELIUM_OS_MAC 1
# if defined( __x86_64__ )
#  define HELIUM_OS_MAC64 1
#  define HELIUM_CPU_X86 1
#  define HELIUM_CPU_X86_64 1
# else
#  define HELIUM_OS_MAC32 1
#  define HELIUM_CPU_X86 1
#  define HELIUM_CPU_X86_32 1
# endif
#elif defined( __gnu_linux__ )
# define HELIUM_OS_LINUX 1
# if defined( __x86_64__ )
#  define HELIUM_CPU_X86 1
#  define HELIUM_CPU_X86_64 1
# else
#  define HELIUM_CPU_X86 1
#  define HELIUM_CPU_X86_32 1
# endif
#else
# error Unsupported platform.
#endif

#if HELIUM_OS_WIN
# define HELIUM_OS_PC 1
#endif

#if defined( _MSC_VER )
# if ( _MSC_FULL_VER < 150030729 )
#  error Helium requires at least VS2008 with SP1 applied to compile.  Please update your compiler.
# endif
# include "Platform/CompilerCl.h"
#elif defined( __clang__ )
# include "Platform/CompilerClang.h"
#elif defined( __GNUC__ )
# include "Platform/CompilerGcc.h"
#else
# error Unsupported compiler.
#endif

#if HELIUM_CPU_X86
# include "Platform/CpuX86.h"
#endif
