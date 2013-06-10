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

int Helium::Execute( const std::string& command )
{
	return system( command.c_str() );
}

int Helium::Execute( const std::string& command, std::string& output )
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

std::string Helium::GetProcessString()
{
	std::ostringstream result;
	result << GetProcessName() << "_" << getpid() << "_" << syscall(SYS_gettid);
	return result.str();
}

std::string Helium::GetProcessPath()
{
	char buf[ PATH_MAX ];
	size_t len = readlink("/proc/self/exe", buf, sizeof(buf));
	buf[len] = '\0';
	return buf;
}

std::string Helium::GetProcessName()
{
	std::string path = GetProcessPath();
	size_t pos = path.find( Helium::PathSeparator );
	if ( pos != std::string::npos )
	{ 
		return path.substr( pos );
	}

	HELIUM_ASSERT( false );
	return "";
}

std::string Helium::GetUserName()
{
	const char* user = getenv( "USER" );
	if ( user )
	{
		return user;
	}
	HELIUM_ASSERT( false );
	return "";
}

std::string Helium::GetMachineName()
{
	const char* user = getenv( "HOSTNAME" );
	if ( user )
	{
		return user;
	}
	HELIUM_ASSERT( false );
	return "";
}

std::string Helium::GetPreferencesDirectory()
{
	const char* home = getenv( "HOME" );
	std::string path = home;
	path += "/.";
	path += GetProcessName();
	path += "/preferences/";
	return path;
}

std::string Helium::GetAppDataDirectory()
{
	const char* home = getenv( "HOME" );
	std::string path = home;
	path += "/.";
	path += GetProcessName();
	path += "/data/";
	return path;
}

std::string Helium::GetDumpDirectory()
{
	const char* home = getenv( "HOME" );
	std::string path = home;
	path += "/.";
	path += GetProcessName();
	path += "/dumps/";
	return path;
}
