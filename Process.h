#pragma once

#include "Platform/API.h"
#include "Platform/Types.h"

namespace Helium
{
	/// Creates a new process with no window or output, use it for running scripts and file association apps
	HELIUM_PLATFORM_API int Execute( const std::string& command );

	/// Creates a new process and captures its standard out and standard error into the passed string
	HELIUM_PLATFORM_API int Execute( const std::string& command, std::string& output );

	/// Get a unique string for this process
	HELIUM_PLATFORM_API std::string GetProcessString();

	/// Get the application path for this process
	HELIUM_PLATFORM_API std::string GetProcessPath();

	/// Get the executable name for this process
	HELIUM_PLATFORM_API std::string GetProcessName();

	/// Get username of the current proess
	HELIUM_PLATFORM_API std::string GetUserName();
	
	/// Get machine name of the current process
	HELIUM_PLATFORM_API std::string GetMachineName();
	
	/// Location for user preferences on disk
	HELIUM_PLATFORM_API std::string GetPreferencesDirectory();

	/// Location for app cache data on disk
	HELIUM_PLATFORM_API std::string GetAppDataDirectory();

	/// Location for crash dumps
	HELIUM_PLATFORM_API std::string GetDumpDirectory();
}