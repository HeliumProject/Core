#include "Precompile.h"
#include "Exception.h"

using namespace Helium;

Helium::Exception::Exception()
{

}

const char* Helium::Exception::What() const
{
    return m_Message.c_str();
}

Exception::Exception( const char *msgFormat, ... )
{
    va_list msgArgs;
    va_start( msgArgs, msgFormat );
    SetMessage( msgFormat, msgArgs );
    va_end( msgArgs );
}

void Exception::SetMessage( const char* msgFormat, ... )
{
    va_list msgArgs;
    va_start( msgArgs, msgFormat );
    SetMessage( msgFormat, msgArgs );
    va_end( msgArgs );
}

void Exception::SetMessage( const char* msgFormat, va_list msgArgs )
{
    char msgBuffer[ERROR_STRING_BUF_SIZE];
    StringPrintArgs( msgBuffer, sizeof(msgBuffer) / sizeof( char ), msgFormat, msgArgs );
    m_Message = msgBuffer;
}