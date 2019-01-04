#include "Precompile.h"
#include "Console.h"

#include "Platform/Assert.h"
#include "Platform/SystemWin.h"

#include <stdlib.h>
#include <io.h>
#include <fcntl.h>

using namespace Helium;

struct ColorEntry
{
	int m_Key;
	int m_Value;
};

ColorEntry g_ColorTable[] =
{
	{ ConsoleColors::None, (int)0xffffffff },
	{ ConsoleColors::Red, FOREGROUND_RED },
	{ ConsoleColors::Green, FOREGROUND_GREEN },
	{ ConsoleColors::Blue, FOREGROUND_BLUE },
	{ ConsoleColors::Yellow, FOREGROUND_RED | FOREGROUND_GREEN, },
	{ ConsoleColors::Aqua, FOREGROUND_GREEN | FOREGROUND_BLUE, },
	{ ConsoleColors::Purple, FOREGROUND_BLUE | FOREGROUND_RED, },
	{ ConsoleColors::White, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, }
};

static inline int LookupColor( ConsoleColor color )
{
	for ( int i=0; i<sizeof(g_ColorTable)/sizeof(g_ColorTable[0]); i++ )
	{
		if ( g_ColorTable[i].m_Key == color )
		{
			return g_ColorTable[i].m_Value;
		}
	}

	HELIUM_ASSERT( false );
	return ConsoleColors::None;
}

int Helium::Scan(const char* fmt, ...)
{
	char buf[1024];
	fgets( buf, sizeof( buf ), stdin );

	va_list args;
	va_start(args, fmt);
	int result = vsscanf( buf, fmt, args );
	va_end(args);
	return result;
}

int Helium::Scan(const wchar_t* fmt, ...)
{
	HELIUM_ASSERT( false );
	return 0;
}

int Helium::ScanArgs(const char* fmt, va_list args)
{
	char buf[1024];
	fgets( buf, sizeof( buf ), stdin );
	return vsscanf( buf, fmt, args );
}

int Helium::ScanArgs(const wchar_t* fmt, va_list args)
{
	HELIUM_ASSERT( false );
	return 0;
}

// vstepano: consider renaming this function to FileLineScan (or something that will hit that this function works on a line by line basis )
int Helium::FileScan(FILE* f, const char* fmt, ...)
{
	HELIUM_ASSERT(f);
	// vstepano: Should we also assert on an empty format string? HELIUM_ASSERT(fmt != '\0');
	char buf[1024];
	fgets( buf, sizeof( buf ), f );

	va_list args;
	va_start(args, fmt);
	int result = vsscanf( buf, fmt, args );
	va_end(args);
	return result;
}

int Helium::FileScan(FILE* f, const wchar_t* fmt, ...)
{
	HELIUM_ASSERT( false );
	return 0;
}

int Helium::FileScanArgs(FILE* f, const char* fmt, va_list args)
{
	HELIUM_ASSERT(f);
	char buf[1024];
	fgets( buf, sizeof( buf ), f );
	return vsscanf( buf, fmt, args );
}

int Helium::FileScanArgs(FILE* f, const wchar_t* fmt, va_list args)
{
	HELIUM_ASSERT( false );
	return 0;
}

int Helium::StringScan(const char* str, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = vsscanf(str, fmt, args);
	va_end(args);
	return result;
}

int Helium::StringScan(const wchar_t* str, const wchar_t* fmt, ...)
{
	HELIUM_ASSERT( false );
	// vstepano: why can't we use vswscanf?
	return 0;
}

int Helium::StringScanArgs(const char* str, const char* fmt, va_list args)
{
	return vsscanf(str, fmt, args);
}

int Helium::StringScanArgs(const wchar_t* str, const wchar_t* fmt, va_list args)
{
	HELIUM_ASSERT( false );
	return 0;
}

int Helium::Print(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = vprintf(fmt, args);
	va_end(args);
	return result;
}

int Helium::Print(const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = vwprintf(fmt, args);
	va_end(args);
	return result;
}

int Helium::PrintArgs(const char* fmt, va_list args)
{
	return vprintf(fmt, args);
}

int Helium::PrintArgs(const wchar_t* fmt, va_list args)
{
	return vwprintf(fmt, args);
}

int Helium::FilePrint(FILE* f, const char* fmt, ...)
{
	HELIUM_ASSERT(f);
	va_list args;
	va_start(args, fmt);
	int result = vfprintf(f, fmt, args);
	va_end(args);
	return result;
}

int Helium::FilePrint(FILE* f, const wchar_t* fmt, ...)
{
	HELIUM_ASSERT(f);
	va_list args;
	va_start(args, fmt);
	int result = vfwprintf(f, fmt, args);
	va_end(args);
	return result;
}

int Helium::FilePrintArgs(FILE* f, const char* fmt, va_list args)
{
	HELIUM_ASSERT(f);
	return vfprintf(f, fmt, args);
}

int Helium::FilePrintArgs(FILE* f, const wchar_t* fmt, va_list args)
{
	HELIUM_ASSERT(f);
	return vfwprintf(f, fmt, args);
}

int Helium::StringPrint(char* dest, size_t destCount, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = vsnprintf(dest, destCount, fmt, args);
	va_end(args);
	return result;
}

int Helium::StringPrint(wchar_t* dest, size_t destCount, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = _vsnwprintf(dest, destCount, fmt, args);
	va_end(args);
	return result;
}

int Helium::StringPrintArgs(char* dest, size_t destCount, const char* fmt, va_list args)
{
	return vsnprintf(dest, destCount, fmt, args);
}

int Helium::StringPrintArgs(wchar_t* dest, size_t destCount, const wchar_t* fmt, va_list args)
{
	return _vsnwprintf(dest, destCount, fmt, args);
}

template< class T >
int PrintArgs(ConsoleColor color, FILE* stream, const T* fmt, va_list args)
{
	CONSOLE_SCREEN_BUFFER_INFO info;

	if (color != ConsoleColors::None)
	{
		// retrieve settings
		GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &info);

		// extract background colors
		int background = info.wAttributes & ~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

		// reset forground only to our desired color
		SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), LookupColor( color ) | FOREGROUND_INTENSITY | background);
	}

	int result = FilePrint(stream, fmt, args);

	fflush(stream);

	if (color != ConsoleColors::None)
	{
		// restore previous settings
		SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), info.wAttributes);
	}

	return result;
}

template< class T >
int PrintString(ConsoleColor color, FILE* stream, const std::basic_string< T >& str)
{
	CONSOLE_SCREEN_BUFFER_INFO info;

	if (color != ConsoleColors::None)
	{
		// retrieve settings
		GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &info);

		// extract background colors
		int background = info.wAttributes & ~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

		// reset forground only to our desired color
		SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), LookupColor( color ) | FOREGROUND_INTENSITY | background);
	}

	int result = FilePrint(stream, "%s", str.c_str());

	fflush(stream);

	if (color != ConsoleColors::None)
	{
		// restore previous settings
		SetConsoleTextAttribute(GetStdHandle(STD_ERROR_HANDLE), info.wAttributes);
	}

	return result;
}

int Helium::Print(ConsoleColor color, FILE* stream, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = ::PrintArgs<char>( color, stream, fmt, args );
	va_end(args);
	return result;
}

int Helium::Print(ConsoleColor color, FILE* stream, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = ::PrintArgs<wchar_t>( color, stream, fmt, args );
	va_end(args);
	return result;
}

int Helium::PrintArgs(ConsoleColor color, FILE* stream, const char* fmt, va_list args)
{
	return ::PrintArgs<char>( color, stream, fmt, args );
}

int Helium::PrintArgs(ConsoleColor color, FILE* stream, const wchar_t* fmt, va_list args)
{
	return ::PrintArgs<wchar_t>( color, stream, fmt, args );
}

int Helium::PrintString(ConsoleColor color, FILE* stream, const std::string& string)
{
	return ::PrintString<char>( color, stream, string );
}

int Helium::PrintString(ConsoleColor color, FILE* stream, const std::wstring& string)
{
	return ::PrintString<wchar_t>( color, stream, string );
}