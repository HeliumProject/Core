#include "PlatformPch.h"
#include "Platform/Process.h"

#include "Platform/Assert.h"
#include "Platform/Error.h"
#include "Platform/Encoding.h"
#include "Platform/Utility.h"

#include <sstream>
#include <shlobj.h>
#include <aclapi.h>

using namespace Helium;

static bool GetEnvVar( wchar_t* var, std::string& value )
{
	DWORD count = ::GetEnvironmentVariable( var, NULL, 0 );
	wchar_t* varValue = (wchar_t*)alloca( count * sizeof( wchar_t ) );
	if ( ::GetEnvironmentVariable( var, varValue, count * sizeof( wchar_t ) ) )
	{
		HELIUM_WIDE_TO_TCHAR( varValue, convertedVarValue );
		value = convertedVarValue;
		return true;
	}

	return false;
}

int Helium::Execute( const std::string& command )
{
	DWORD result = 0;

	STARTUPINFO si;
	memset( &si, 0, sizeof(si) );
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	memset( &pi, 0, sizeof( pi ) );

	HELIUM_TCHAR_TO_WIDE( command.c_str(), convertedCommand );

	// Start the child process.
	if( !CreateProcess(
		NULL,             // No module name (use command line)
		convertedCommand, // Command line
		NULL,             // Process handle not inheritable
		NULL,             // Thread handle not inheritable
		FALSE,            // Set handle inheritance to FALSE
		NULL,             // Creation flags
		NULL,             // Use parent's environment block
		NULL,             // Use parent's starting directory
		&si,              // Pointer to STARTUPINFO structure
		&pi ) )           // Pointer to PROCESS_INFORMATION structure
	{
		return -1;
	}

	DWORD waitResult = ::WaitForSingleObject( pi.hProcess, INFINITE );
	HELIUM_ASSERT( waitResult != WAIT_FAILED );

	BOOL codeResult = ::GetExitCodeProcess( pi.hProcess, &result );
	HELIUM_ASSERT( codeResult );

	::CloseHandle( pi.hProcess );
	::CloseHandle( pi.hThread );

	return result;
}

int Helium::Execute( const std::string& command, std::string& output )
{
	HANDLE hReadPipe;
	HANDLE hWritePipe;

	SECURITY_ATTRIBUTES sa;
	memset( &sa, 0, sizeof( sa ) );
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if( !CreatePipe( &hReadPipe, &hWritePipe, &sa, 0 ) )
	{
		return -1;
	}

	STARTUPINFO si;
	memset( &si, 0, sizeof(si) );
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.hStdOutput = hWritePipe;
	si.hStdError = hWritePipe;      

	PROCESS_INFORMATION  pi;
	memset( &pi, 0, sizeof( pi ) );

	HELIUM_TCHAR_TO_WIDE( command.c_str(), convertedCommand );

	if( !::CreateProcess(
		NULL,             // filename
		convertedCommand, // command line for child
		NULL,             // process security descriptor
		NULL,             // thread security descriptor
		TRUE,             // inherit handles?
		NULL,             // creation flags
		NULL,             // inherited environment address
		NULL,             // startup dir; NULL = start in current
		&si,              // pointer to startup info (input)
		&pi ) )           // pointer to process info (output)
	{
		::CloseHandle( hReadPipe );
		::CloseHandle( hWritePipe );
		return -1;
	}

	// close the write end of the pipe so the child will terminate it with EOF
	::CloseHandle( hWritePipe );

	// read from the pipe until EOF condition reached
	char buffer[80];
	unsigned long count;
	std::stringstream stream;
	BOOL success = TRUE;
	do
	{
		while ( success = ReadFile( hReadPipe, buffer, sizeof(buffer), &count, NULL ) )
		{
			if( success )
			{
				stream.write( buffer, count );
			}
			else
			{
				if ( ::GetLastError() == ERROR_BROKEN_PIPE )
				{
					break;
				}
				else
				{
					return -1;
				}
			}
		}
	} while( success && count );

	// done reading, close our read pipe
	::CloseHandle( hReadPipe );

	// copy output string
	output = stream.str();

	// get exit code
	DWORD result = 0;
	BOOL codeResult = ::GetExitCodeProcess( pi.hProcess, &result );
	HELIUM_ASSERT( codeResult );

	// close the process handle
	::CloseHandle( pi.hProcess );
	::CloseHandle( pi.hThread );

	return result;
}

ProcessHandle Helium::Spawn( const std::string& command, bool autoKill )
{
	static HANDLE hJob = INVALID_HANDLE_VALUE;	
	if ( autoKill && hJob == INVALID_HANDLE_VALUE )
	{
		hJob = CreateJobObject( NULL, NULL ); // GLOBAL
		if( HELIUM_VERIFY( hJob ) )
		{
			// Configure all child processes associated with the job to terminate when the
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
			jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			HELIUM_ASSERT( SetInformationJobObject( hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)) );
		}
	}

	STARTUPINFO si;
	memset( &si, 0, sizeof( si ) );
	si.cb = sizeof( si );

	PROCESS_INFORMATION pi;
	memset( &pi, 0, sizeof( pi ) );

	DWORD flags = 0x0;

	OSVERSIONINFO osvi;
	memset(&osvi, 0, sizeof( OSVERSIONINFO ) );
	osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
	::GetVersionEx(&osvi);
	uint32_t version = 0x0;
	version |= osvi.dwMajorVersion * 10;
	version |= osvi.dwMinorVersion;

	if ( version >= 60 && version <= 61 ) // vista and 7
	{
		// windows vista and beyond somtimes have system software that attach child processes to jobs,
		//  and pre-windows 8 you can only attach a process to a single job
		flags |= CREATE_BREAKAWAY_FROM_JOB;
	}

#ifdef HELIUM_DEBUG
	flags |= CREATE_NEW_CONSOLE;
#else
	flags |= CREATE_NO_WINDOW;
#endif

	ProcessHandle handle = InvalidProcessHandle;

	HELIUM_TCHAR_TO_WIDE( command.c_str(), convertedCommand );

	if ( ::CreateProcess(
		NULL,             // filename
		convertedCommand, // command line for child
		NULL,             // process security descriptor
		NULL,             // thread security descriptor
		FALSE,            // inherit handles?
		flags,            // creation flags
		NULL,             // inherited environment address
		NULL,             // startup dir; NULL = start in current
		&si,              // pointer to startup info (input)
		&pi ) )           // pointer to process info (output)
	{
		handle = pi.hProcess;

		if ( autoKill && hJob )
		{
			HELIUM_ASSERT( ::AssignProcessToJobObject( hJob, pi.hProcess ) );
		}

		// release handles to our new process
		::CloseHandle( pi.hThread );
	}

	return handle;
}

bool Helium::SpawnRunning( ProcessHandle handle )
{
	DWORD code = 0x0;
	::GetExitCodeProcess( handle, &code );
	return code == STILL_ACTIVE;
}

int Helium::SpawnResult( ProcessHandle handle )
{
	DWORD code = 0x0;
	::WaitForSingleObject( handle, INFINITE );
	::GetExitCodeProcess( handle, &code );
	::CloseHandle( handle );
	return code;
}

void Helium::SpawnKill( ProcessHandle handle )
{
	::TerminateProcess( handle, -1 );	
}

int Helium::GetProcessId( ProcessHandle handle )
{
	if ( handle == InvalidProcessHandle )
	{
		return ::GetCurrentProcessId();
	}
	else
	{
		return ::GetProcessId( handle );
	}
}

std::string Helium::GetProcessString()
{
	std::ostringstream result;
	result << GetProcessName() << "_" << GetCurrentProcessId() << "_" << GetCurrentThreadId();
	return result.str();
}

std::string Helium::GetProcessPath()
{
	HMODULE moduleHandle = ::GetModuleHandle( NULL );

	wchar_t module[ MAX_PATH ];
	DWORD result = ::GetModuleFileName( moduleHandle, module, MAX_PATH );
	HELIUM_ASSERT( result );
	HELIUM_UNREF( result );

	HELIUM_WIDE_TO_TCHAR( module, convertedModule );
	return convertedModule;
}

std::string Helium::GetProcessName()
{
	HMODULE moduleHandle = GetModuleHandle( NULL );

	wchar_t module[ MAX_PATH ];
	::GetModuleFileName( moduleHandle, module, MAX_PATH );

	wchar_t file[ MAX_PATH ];
	_wsplitpath( module, NULL, NULL, file, NULL );

	HELIUM_WIDE_TO_TCHAR( file, convertedFile );
	return convertedFile;
}

std::string Helium::GetUserName()
{
	std::string username;
	HELIUM_VERIFY( GetEnvVar( L"USERNAME", username ) );
	return username;
}

std::string Helium::GetMachineName()
{
	std::string computername;
	HELIUM_VERIFY( GetEnvVar( L"COMPUTERNAME", computername ) );
	return computername;
}

std::string Helium::GetPreferencesDirectory()
{
	std::string profileDirectory;

	wchar_t path[ MAX_PATH ];
	HRESULT result = SHGetFolderPath( NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, path );
	bool bSuccess = ( result == S_OK );
	if ( bSuccess )
	{
		HELIUM_WIDE_TO_TCHAR( path, convertedPath );
		profileDirectory = convertedPath;
	}

	return profileDirectory;
}

std::string Helium::GetAppDataDirectory()
{
	std::string appDataDirectory;

	wchar_t path[ MAX_PATH ];
	HRESULT result = SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path );
	bool bSuccess = ( result == S_OK );
	if ( bSuccess )
	{
		HELIUM_WIDE_TO_TCHAR( path, convertedPath );
		appDataDirectory = convertedPath;
	}

	return appDataDirectory;
}

std::string Helium::GetDumpDirectory()
{
	wchar_t tempDir[ MAX_PATH ];
	size_t result = ::GetTempPath( MAX_PATH, tempDir );
	if ( result == 0 || result > MAX_PATH )
	{
		return TXT("");
	}

	// Make sure that the directory exists
	wchar_t directory[ MAX_PATH ] = { 0 };
	_snwprintf( directory, sizeof( directory ) - 1, L"%s\\dumps", &tempDir );

	HELIUM_WIDE_TO_TCHAR( directory, convertedDirectory );
	return convertedDirectory;
}