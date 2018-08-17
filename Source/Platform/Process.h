#pragma once

#include "Platform/API.h"
#include "Platform/Types.h"

#if HELIUM_OS_WIN
# define HELIUM_INVALID_PROCESS ( NULL )
# define HELIUM_INVALID_MODULE ( NULL )
# define HELIUM_MODULE_EXTENSION "dll"
#else // HELIUM_OS_WIN
# define HELIUM_INVALID_PROCESS ( 0 )
# define HELIUM_INVALID_MODULE ( NULL )
# if HELIUM_OS_MAC
#  define HELIUM_MODULE_EXTENSION "dylib"
# elif HELIUM_OS_LINUX
#  define HELIUM_MODULE_EXTENSION "so"
# else
#  error Unknown module extension!
# endif
#endif // HELIUM_OS_WIN

namespace Helium
{
#if HELIUM_OS_WIN
	typedef void* ProcessHandle;
	typedef void* ModuleHandle;
#else
	typedef pid_t ProcessHandle;
	typedef void* ModuleHandle;
#endif

	/// Creates a new process with no window or output, use it for running scripts and file association apps
	HELIUM_PLATFORM_API int Execute( const std::string& command );

	/// Creates a new process and captures its standard out and standard error into the passed string
	HELIUM_PLATFORM_API int Execute( const std::string& command, std::string& output );

	/// Spawn another process and run it alongside the executing process
	HELIUM_PLATFORM_API ProcessHandle Spawn( const std::string& command, bool autoKill = false );

	/// Test if the spawned process is running
	HELIUM_PLATFORM_API bool SpawnRunning( ProcessHandle handle );

	/// Get result of spawned process and release any behinds-the-scenes resources
	///  (always call!, even for fire-and-forget child processes)
	HELIUM_PLATFORM_API int SpawnResult( ProcessHandle handle );

	/// Forcefully terminate spawn
	HELIUM_PLATFORM_API void SpawnKill( ProcessHandle handle );

	/// Get the process id
	HELIUM_PLATFORM_API int GetProcessId( ProcessHandle handle = HELIUM_INVALID_PROCESS );

	/// Get a unique string for this process
	HELIUM_PLATFORM_API std::string GetProcessString();

	/// Get the application path for this process
	HELIUM_PLATFORM_API std::string GetProcessPath();

	/// Get the executable name for this process
	HELIUM_PLATFORM_API std::string GetProcessName();

	/// Get the current working path
	HELIUM_PLATFORM_API std::string GetCurrentWorkingPath();

	/// Get username of the current proess
	HELIUM_PLATFORM_API std::string GetUserName();
	
	/// Get machine name of the current process
	HELIUM_PLATFORM_API std::string GetMachineName();
	
	/// Location for user preferences on disk
	HELIUM_PLATFORM_API std::string GetHomeDirectory();

	/// Load a module into the caller's address space
	HELIUM_PLATFORM_API ModuleHandle LoadModule( const char* modulePath );

	/// Unload a module from the caller's address space
	HELIUM_PLATFORM_API void UnloadModule( ModuleHandle handle );

	/// Find a function in a loaded module
	HELIUM_PLATFORM_API void* GetModuleFunction( ModuleHandle handle, const char* functionName );
}
