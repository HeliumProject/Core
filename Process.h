#pragma once

#include "Platform/API.h"
#include "Platform/Types.h"

namespace Helium
{
#if HELIUM_OS_WIN
	typedef HANDLE ProcessHandle;
	const static HANDLE InvalidProcessHandle = NULL;
#else
	typedef pid_t ProcessHandle;
	const static pid_t InvalidProcessHandle = 0;
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
	HELIUM_PLATFORM_API int GetProcessId( ProcessHandle handle = InvalidProcessHandle );

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
}
