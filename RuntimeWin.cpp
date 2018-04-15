#include "Precompile.h"
#include "Runtime.h"

#include <eh.h>
#include <new.h>
#include <stdlib.h>
#include <sstream>

using namespace Helium;
using namespace Helium::Platform;

Platform::Type Platform::GetType()
{
    return Types::Windows;
}

static int NewHandler( size_t size )
{
    std::ostringstream str;
    str << "Failed to allocate " << size << " bytes";
    throw std::exception ( str.str().c_str() );
}

static void PureCallHandler()
{
    throw std::exception ("A pure virtual function was called");
}

static void InvalidParameterHandler( const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t /*pReserved*/ )
{
#ifdef _DEBUG
    static const wchar_t unknown[] = L"Unknown"; // this is *probably* overkill, but just for insurance

    char ex[ 256 ];
    wcstombs( ex, expression ? expression : unknown, sizeof(ex) );
    char fn[ 256 ];
    wcstombs( fn, function ? function : unknown, sizeof(fn) );
    char fl[ 256 ];
    wcstombs( fl, file ? file : unknown, sizeof(fl) );

    std::ostringstream str;
    str << "An invalid parameter was passed: ";
    str << "Expression: " << ex;
    str << "Function: " << fn;
    str << "File: " << fl;
    str << "Line: " << line;
    throw std::exception ( str.str().c_str() );
#else
    throw std::exception ("An invalid parameter was passed");
#endif
}

void Helium::EnableCPPErrorHandling( bool enable )
{
    if ( enable )
    {
        _set_new_handler( &NewHandler );
    }
    else if ( _query_new_handler() == &NewHandler )
    {
        _set_new_handler( NULL );
    }

    if ( enable )
    {
        _set_purecall_handler( &PureCallHandler );
    }
    else if ( _get_purecall_handler() == &PureCallHandler )
    {
        _set_purecall_handler( NULL );
    }

    if ( enable )
    {
        _set_invalid_parameter_handler( &InvalidParameterHandler );
    }
    else if ( _get_invalid_parameter_handler() == &InvalidParameterHandler )
    {
        _set_invalid_parameter_handler( NULL );
    }
}
