#include "Precompile.h"
#include "Exception.h"

#if HELIUM_OS_WIN
# include "Platform/SystemWin.h"
#endif

#include "Platform/Assert.h"
#include "Platform/Encoding.h"
#include "Platform/Console.h"

#include "Foundation/Log.h"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sys/stat.h>

using namespace Helium;

BreakpointSignature::Delegate   Helium::g_BreakpointOccurred;
ExceptionSignature::Delegate    Helium::g_ExceptionOccurred;
TerminateSignature::Event       Helium::g_Terminating;

bool g_EnableExceptionFilter = false;

#if HELIUM_OS_WIN

static LONG __stdcall ProcessFilteredException( LPEXCEPTION_POINTERS info )
{
    return Helium::ProcessException(info, true, true);
}

#endif

void Helium::EnableExceptionFilter(bool enable)
{
#if HELIUM_OS_WIN

    g_EnableExceptionFilter = enable;

    // handles an exception occuring in the process not handled by a user exception handler
    SetUnhandledExceptionFilter( g_EnableExceptionFilter ? &ProcessFilteredException : NULL );

#endif
}

void Helium::ProcessException(const Helium::Exception& exception, bool print, bool fatal)
{
#if HELIUM_OS_WIN
    SetUnhandledExceptionFilter( NULL );
#endif

    ExceptionArgs args( ExceptionTypes::CPP, fatal );

    const char* cppClass = "Unknown";
#ifdef _CPPRTTI
	try
    {
        cppClass = typeid(exception).name();
    }
    catch (...)
    {
    }
#else
	cppClass = "Unknown, no RTTI";
#endif

    bool converted = Helium::ConvertString( exception.What(), args.m_Message );
    HELIUM_ASSERT( converted );

    converted = Helium::ConvertString( cppClass, args.m_CPPClass );
    HELIUM_ASSERT( converted );

    args.m_State = Log::GetOutlineState();

    if (print)
    {
        Helium::Print(Helium::ConsoleColors::Red, stderr, "An exception has occurred\nType:    C++ Exception\n Class:   %s\n Message: %s\n", args.m_CPPClass.c_str(), args.m_Message.c_str() );
    }

    g_ExceptionOccurred.Invoke( args );

    if ( fatal )
    {
        g_Terminating.Raise( TerminateArgs () );
        EnableExceptionFilter( false );
    }
    else if ( g_EnableExceptionFilter )
    {
#if HELIUM_OS_WIN
        SetUnhandledExceptionFilter( &ProcessFilteredException );
#endif
    }
}

void Helium::ProcessException(const std::exception& exception, bool print, bool fatal)
{
#if HELIUM_OS_WIN
    SetUnhandledExceptionFilter( NULL );
#endif

    ExceptionArgs args( ExceptionTypes::CPP, fatal );

    const char* cppClass = "Unknown";
#ifdef _CPPRTTI
    try
    {
        cppClass = typeid(exception).name();
    }
    catch (...)
    {
    }
#else
	cppClass = "Unknown, no RTTI";
#endif

    bool converted = Helium::ConvertString( exception.what(), args.m_Message );
    HELIUM_ASSERT( converted );

    converted = Helium::ConvertString( cppClass, args.m_CPPClass );
    HELIUM_ASSERT( converted );

    args.m_State = Log::GetOutlineState();

    if (print)
    {
        Helium::Print(Helium::ConsoleColors::Red, stderr, "An exception has occurred\nType:    C++ Exception\n Class:   %s\n Message: %s\n", args.m_CPPClass.c_str(), args.m_Message.c_str() );
    }

    g_ExceptionOccurred.Invoke( args );

    if ( fatal )
    {
        g_Terminating.Raise( TerminateArgs () );
        EnableExceptionFilter( false );
    }
    else if ( g_EnableExceptionFilter )
    {
#if HELIUM_OS_WIN
        SetUnhandledExceptionFilter( &ProcessFilteredException );
#endif
    }
}

#if HELIUM_OS_WIN

uint32_t Helium::ProcessException(LPEXCEPTION_POINTERS info, bool print, bool fatal)
{
    SetUnhandledExceptionFilter( NULL );

    // handle breakpoint exceptions outside the debugger
    if ( !::IsDebuggerPresent()
        && info->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT
        && g_BreakpointOccurred.Valid() )
    {
        BreakpointArgs args (info, fatal);
        g_BreakpointOccurred.Invoke( args );
        return args.m_Result;
    }
    else
    {
        ExceptionArgs args ( ExceptionTypes::Structured, fatal );

        args.m_State = Log::GetOutlineState();

        Helium::GetExceptionDetails( info, args );

        if ( print )
        {
            Helium::Print( Helium::ConsoleColors::Red, stderr, "%s", GetExceptionInfo( info ).c_str() );
        }

        g_ExceptionOccurred.Invoke( args );

        if ( fatal )
        {
            g_Terminating.Raise( TerminateArgs () );
            EnableExceptionFilter( false );
        }
    }

    if ( g_EnableExceptionFilter )
    {
        SetUnhandledExceptionFilter( &ProcessFilteredException );
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

#endif