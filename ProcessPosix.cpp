#include "PlatformPch.h"
#include "Platform/Process.h"

#include "Platform/Assert.h"
#include "Platform/Error.h"
#include "Platform/Encoding.h"
#include "Platform/Utility.h"
#include "Platform/File.h"

#include <sstream>

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

using namespace Helium;

int Helium::Execute( const tstring& command )
{
	return system( command.c_str() );
}

int Helium::Execute( const tstring& command, tstring& output )
{
	FILE* f = popen( command.c_str(), "r" );

	char line[200];
	line[199] = '\0';
	while ( fgets(line, 199, f) )
	{
		output += line;
	}

	return pclose( f );
}

tstring Helium::GetProcessString()
{
	tostringstream result;
	result << GetProcessName() << "_" << getpid() << "_" << syscall(SYS_gettid);
	return result.str();
}

tstring Helium::GetProcessPath()
{
    char buf[ PATH_MAX ];
    readlink("/proc/self/exe", buf, sizeof(buf));
	return buf;
}

tstring Helium::GetProcessName()
{
	tstring path = GetProcessPath();
	size_t pos = path.find( Helium::PathSeparator );
	if ( pos != tstring::npos )
	{ 
		return path.substr( pos );
	}

	HELIUM_ASSERT( false );
	return "";
}

tstring Helium::GetUserName()
{
	const char* user = getenv( "USER" );
	if ( user )
	{
		return user;
	}
	HELIUM_ASSERT( false );
	return "";
}

tstring Helium::GetMachineName()
{
	const char* user = getenv( "HOSTNAME" );
	if ( user )
	{
		return user;
	}
	HELIUM_ASSERT( false );
	return "";
}

tstring Helium::GetPreferencesDirectory()
{
	const char* home = getenv( "HOME" );
	tstring path = home;
	path += "/.";
	path += GetProcessName();
	path += "/preferences/";
	return path;
}

tstring Helium::GetAppDataDirectory()
{
	const char* home = getenv( "HOME" );
	tstring path = home;
	path += "/.";
	path += GetProcessName();
	path += "/data/";
	return path;
}

tstring Helium::GetDumpDirectory()
{
	const char* home = getenv( "HOME" );
	tstring path = home;
	path += "/.";
	path += GetProcessName();
	path += "/dumps/";
	return path;
}
