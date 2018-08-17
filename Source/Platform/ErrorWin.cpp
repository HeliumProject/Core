#include "Precompile.h"
#include "Error.h"

#include "Platform/Encoding.h"
#include "Platform/SystemWin.h"

using namespace Helium;

uint32_t Helium::GetLastError()
{
	return ::GetLastError();
}

std::string Helium::GetErrorString( uint32_t errorOverride )
{
	// get the system error
	DWORD error = ( errorOverride != 0 ) ? errorOverride : ::GetLastError();

	LPTSTR lpMsgBuf = NULL;
	::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		lpMsgBuf, 0, NULL );

	std::wstring result;
	if (lpMsgBuf)
	{
		result = lpMsgBuf;
		::LocalFree( lpMsgBuf );
	}
	else
	{
		result = L"Unknown error (the error code could not be translated)";
	}

	// trim enter chracters from message
	while (!result.empty() && (*result.rbegin() == '\n' || *result.rbegin() == '\r'))
	{
		result.resize( result.size() - 1 );
	}

	std::string str;
	ConvertString( result, str );
	return str;
}
