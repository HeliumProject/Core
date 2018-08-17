#pragma once

#ifdef _WINDOWS_
#error Windows already included
#endif

#ifdef WINVER
#error WINVER defined
#endif

#ifdef _WIN32_IE
#error _WIN32_IE defined
#endif

#ifdef _WIN32_WINNT
#error _WIN32_WINNT defined
#endif

#ifdef WIN32_LEAN_AND_MEAN
#error WIN32_LEAN_AND_MEAN defined
#endif

// prevent min/max from being #defined
#define NOMINMAX

// prevent winsock1 from being included
#define _WINSOCKAPI_

// prevent infrequently used stuff
#define WIN32_LEAN_AND_MEAN

// windows 7
#define WINVER			0x0601
#define _WIN32_WINNT	0x0601

// internet explorer 7
#define _WIN32_IE		0x0700

#include <windows.h>
#include <winsock2.h>
#include <intrin.h>
