#include "Precompile.h"
#include "Console.h"

#include "Platform/Assert.h"

#include <stdlib.h>

using namespace Helium;

struct ColorEntry
{
	int m_Key;
	const char* m_Value;
};

ColorEntry g_ColorTable[] =
{
	{ ConsoleColors::None,   "\033[0m" },
	{ ConsoleColors::Red, 	 "\033[1;31m" },
	{ ConsoleColors::Green,  "\033[1;32m" },
	{ ConsoleColors::Blue, 	 "\033[1;34m" },
	{ ConsoleColors::Yellow, "\033[1;33m" },
	{ ConsoleColors::Aqua,   "\033[1;36m" },
	{ ConsoleColors::Purple, "\033[1;35m" },
	{ ConsoleColors::White,  "\033[1;37m" }
};

static inline const char* LookupColor( ConsoleColor color )
{
	for ( int i=0; i<sizeof(g_ColorTable)/sizeof(g_ColorTable[0]); i++ )
	{
		if ( g_ColorTable[i].m_Key == color )
		{
			return g_ColorTable[i].m_Value;
		}
	}

	HELIUM_ASSERT( false );
	return "\033[0m";
}

int Helium::Scan(const char* fmt, ...)
{
	char buf[1024];
	if ( fgets( buf, sizeof( buf ), stdin ) )
	{
		va_list args;
		va_start(args, fmt);
		int result = vsscanf( buf, fmt, args );
		va_end(args);
		return result;
	}
	else
	{
		return 0;
	}
}

int Helium::Scan(const wchar_t* fmt, ...)
{
	HELIUM_ASSERT( false );
	return 0;
}

int Helium::ScanArgs(const char* fmt, va_list args)
{
	char buf[1024];
	if ( fgets( buf, sizeof( buf ), stdin ) )
	{
		return vsscanf( buf, fmt, args );
	}
	else
	{
		return 0;
	}
}

int Helium::ScanArgs(const wchar_t* fmt, va_list args)
{
	HELIUM_ASSERT( false );
	return 0;
}

int Helium::FileScan(FILE* f, const char* fmt, ...)
{
	char buf[1024];
	if ( fgets( buf, sizeof( buf ), f ) )
	{
		va_list args;
		va_start(args, fmt);
		int result = vsscanf( buf, fmt, args );
		va_end(args);
		return result;
	}
	else
	{
		return 0;
	}
}

int Helium::FileScan(FILE* f, const wchar_t* fmt, ...)
{
	HELIUM_ASSERT( false );
	return 0;
}

int Helium::FileScanArgs(FILE* f, const char* fmt, va_list args)
{
	char buf[1024];
	if ( fgets( buf, sizeof( buf ), f ) )
	{
		return vsscanf( buf, fmt, args );
	}
	else
	{
		return 0;
	}
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
	va_list args;
	va_start(args, fmt);
	int result = vfprintf(f, fmt, args);
	va_end(args);
	return result;
}

int Helium::FilePrint(FILE* f, const wchar_t* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = vfwprintf(f, fmt, args);
	va_end(args);
	return result;
}

int Helium::FilePrintArgs(FILE* f, const char* fmt, va_list args)
{
	return vfprintf(f, fmt, args);
}

int Helium::FilePrintArgs(FILE* f, const wchar_t* fmt, va_list args)
{
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
	int result = vswprintf(dest, destCount, fmt, args);
	va_end(args);
	return result;
}

int Helium::StringPrintArgs(char* dest, size_t destCount, const char* fmt, va_list args)
{
	return vsnprintf(dest, destCount, fmt, args);
}

int Helium::StringPrintArgs(wchar_t* dest, size_t destCount, const wchar_t* fmt, va_list args)
{
	return vswprintf(dest, destCount, fmt, args);
}

template< class T >
int PrintArgs(ConsoleColor color, FILE* stream, const T* fmt, va_list args)
{
	if (color != ConsoleColors::None)
	{
		FilePrint( stream, LookupColor( color ) );
	}

	int result = FilePrint(stream, fmt, args);

	if (color != ConsoleColors::None)
	{
		HELIUM_ASSERT( g_ColorTable[0].m_Key == ConsoleColors::None );
		FilePrint( stream, g_ColorTable[0].m_Value );
	}

	fflush(stream);

	return result;
}

template< class T >
int PrintString(ConsoleColor color, FILE* stream, const std::basic_string< T >& tstring)
{
	if (color != ConsoleColors::None)
	{
		FilePrint( stream, LookupColor( color ) );
	}

	int result = FilePrint(stream, "%s", tstring.c_str());

	if (color != ConsoleColors::None)
	{
		HELIUM_ASSERT( g_ColorTable[0].m_Key == ConsoleColors::None );
		FilePrint( stream, g_ColorTable[0].m_Value );
	}

	fflush(stream);

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