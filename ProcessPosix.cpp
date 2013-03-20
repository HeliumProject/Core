#include "Platform/Process.h"
#include "Platform/Assert.h"

int Helium::Execute( const tstring& command, bool showWindow, bool block )
{
    HELIUM_BREAK();
    return -1;
}

int Helium::Execute( const tstring& command, tstring& output, bool showWindow )
{
    HELIUM_BREAK();
    return -1;
}

tstring Helium::GetProcessString()
{
    HELIUM_BREAK();
    return TXT( "" );
}

/*
// not implemented, and not used anywhere else in helium
tstring Helium::GetCrashdumpDirectory()
{
    HELIUM_BREAK();
    return TXT("");
}
*/
