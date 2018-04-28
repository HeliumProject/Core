#include "Precompile.h"
#include "Process.h"

#include "Platform/Assert.h"
#include "Platform/Error.h"
#include "Platform/Encoding.h"
#include "Platform/Utility.h"
#include "Platform/File.h"

#include <sstream>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <signal.h>
#include <spawn.h>
#include <dlfcn.h>
#include <pwd.h>

#if HELIUM_OS_MAC
# include <mach-o/dyld.h>
#endif

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

ProcessHandle Helium::Spawn( const std::string& cmd, bool autoKill )
{
	ProcessHandle pid;

	if ( autoKill )
	{
		// NYI: http://stackoverflow.com/questions/284325/how-to-make-child-process-die-after-parent-exits
		return 0;
	}
	else
	{
		const char* spawnedArgs[] = { "/bin/bash", "-c", cmd.c_str(), NULL };
		if ( posix_spawn( &pid, spawnedArgs[0], NULL, NULL, const_cast< char* const* >( spawnedArgs ), NULL ) == 0 )
		{
			return pid;
		}
	}

	return 0;
}

bool Helium::SpawnRunning( ProcessHandle handle )
{
	int status = 0;
	HELIUM_VERIFY( handle == waitpid( handle, &status, WNOHANG | WUNTRACED ) );
	if ( WIFEXITED( status ) )
	{
		return false;
	}
	else
	{
		return true;
	}
}

int Helium::SpawnResult( ProcessHandle handle )
{
	int status = 0;
	HELIUM_VERIFY( handle == waitpid( handle, &status, WUNTRACED ) );
	if ( WIFEXITED( status ) )
	{
		return WEXITSTATUS( status );
	}
	return -1;
}

void Helium::SpawnKill( ProcessHandle handle )
{
	kill( handle, SIGTERM );	
}

int Helium::GetProcessId( ProcessHandle handle )
{
	return handle;
}

std::string Helium::GetProcessString()
{
	std::ostringstream result;
#if HELIUM_OS_MAC
	uint64_t tid;
	pthread_threadid_np(NULL, &tid);
	result << GetProcessName() << "_" << getpid() << "_" << tid;
#else
	result << GetProcessName() << "_" << getpid() << "_" << syscall(SYS_gettid);
#endif
	return result.str();
}

std::string Helium::GetProcessPath()
{
	char buf[ PATH_MAX ];

#if HELIUM_OS_MAC
	uint32_t size = sizeof(buf);
	HELIUM_ASSERT( _NSGetExecutablePath(buf, &size) == 0 );
#elif HELIUM_OS_LINUX
	size_t len = readlink("/proc/self/exe", buf, sizeof(buf));
	buf[len] = '\0';
#endif

	return buf;
}

std::string Helium::GetProcessName()
{
	std::string path = GetProcessPath();
	size_t pos = path.rfind( Helium::PathSeparator );
	if ( pos != std::string::npos )
	{ 
		return path.substr( pos + 1 );
	}

	HELIUM_ASSERT( false );
	return "";
}

std::string Helium::GetCurrentWorkingPath()
{
	char buf[ PATH_MAX ];

	if (getcwd(buf, PATH_MAX))
	{
		return std::string(buf);
	}
	else
	{
		return std::string("");
	}
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

std::string Helium::GetHomeDirectory()
{
	const char* home = getenv( "HOME" );

	if ( !home )
	{
		struct passwd *pwd = getpwuid(getuid());
		if ( pwd )
		{
			home = pwd->pw_dir;
		}
	}

	HELIUM_ASSERT( home );
	return home;
}

Helium::ModuleHandle Helium::LoadModule( const char* modulePath )
{
	return dlopen( modulePath, RTLD_NOW | RTLD_LOCAL );
}

void Helium::UnloadModule( ModuleHandle handle )
{
	if ( handle != HELIUM_INVALID_MODULE )
	{
		HELIUM_VERIFY( dlclose( handle ) );
	}
}

void* Helium::GetModuleFunction( ModuleHandle handle, const char* functionName )
{
	return dlsym( handle, functionName );
}
