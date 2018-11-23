#include "Precompile.h"
#include "File.h"

#include "Platform/SystemWin.h"
#include "Platform/Assert.h"
#include "Platform/Encoding.h"
#include "Platform/Types.h"

#include <vector>
#include <sys/stat.h>

#undef CreateDirectory
#undef DeleteDirectory
#undef CopyFile
#undef MoveFile
#undef DeleteFile

using namespace Helium;

static uint64_t FromWindowsTime( FILETIME time )
{
	// FILETIME is a 64-bit unsigned integer representing
	// the number of 100-nanosecond intervals since January 1, 1601
	// UNIX timestamp is number of seconds since January 1, 1970
	// 116444736000000000 = 10000000 * 60 * 60 * 24 * 365 * 369 + 89 leap days
	uint64_t ticks = ( (uint64_t)time.dwHighDateTime << 32 ) | time.dwLowDateTime;
	return (ticks - 116444736000000000) / 10000000;
}

static void FromWindowsAttributes( DWORD attrs, uint32_t& mode )
{
	mode = 0;
	mode |= ( attrs & FILE_ATTRIBUTE_READONLY ) ? StatusModes::Read : ( StatusModes::Read | StatusModes::Write );
	mode |= ( attrs & FILE_ATTRIBUTE_DIRECTORY ) ? StatusModes::Directory : StatusModes::None;
	mode |= ( attrs & FILE_ATTRIBUTE_REPARSE_POINT ) ? StatusModes::Link : StatusModes::None;
	mode |= ( attrs & FILE_ATTRIBUTE_DEVICE ) ? StatusModes::Special : StatusModes::None;
	mode |= ( attrs & FILE_ATTRIBUTE_SYSTEM ) ? StatusModes::Special : StatusModes::None;
}

static void FromWindowsFindData( const WIN32_FIND_DATA& windowsFile, DirectoryEntry& ourFile )
{
	ConvertString( windowsFile.cFileName, ourFile.m_Name );

	ourFile.m_Stat.m_Size = ( (uint64_t)windowsFile.nFileSizeHigh << 32 ) | windowsFile.nFileSizeLow;
	ourFile.m_Stat.m_CreatedTime = FromWindowsTime( windowsFile.ftCreationTime );
	ourFile.m_Stat.m_ModifiedTime = FromWindowsTime( windowsFile.ftLastWriteTime );
	ourFile.m_Stat.m_AccessTime = FromWindowsTime( windowsFile.ftLastAccessTime );

	FromWindowsAttributes( windowsFile.dwFileAttributes, ourFile.m_Stat.m_Mode );
}

//
// File contents
//

File::File()
	: m_Handle( INVALID_HANDLE_VALUE )
{
}

File::~File()
{
	HELIUM_VERIFY( Close() );
}

bool File::IsOpen() const
{
	return m_Handle != INVALID_HANDLE_VALUE;
}

bool File::Open( const char* filename, FileMode mode, bool truncate )
{
	DWORD desiredAccess = 0;
	if( mode & FileModes::Read )
	{
		desiredAccess |= GENERIC_READ;
	}

	if( mode & FileModes::Write )
	{
		desiredAccess |= GENERIC_WRITE;
	}

	// Allow other files to read if we are not writing to the file.
	DWORD shareMode = 0;
	if( !( mode & FileModes::Write ) )
	{
		shareMode |= FILE_SHARE_READ;
	}

	DWORD createDisposition = OPEN_EXISTING;
	if( mode & FileModes::Write )
	{
		createDisposition = ( truncate ? CREATE_ALWAYS : OPEN_ALWAYS );
	}

	HELIUM_TCHAR_TO_WIDE( filename, convertedFilename );
	m_Handle = ::CreateFile( convertedFilename, desiredAccess, shareMode, NULL, createDisposition, FILE_ATTRIBUTE_NORMAL, NULL );
	return m_Handle != INVALID_HANDLE_VALUE;
}

bool File::Close()
{
	if ( IsOpen() && !::CloseHandle( m_Handle ) )
	{
		return false;
	}

	m_Handle = INVALID_HANDLE_VALUE;
	return true;
}

bool File::Read( void* buffer, size_t numberOfBytesToRead, size_t* numberOfBytesRead )
{
	HELIUM_ASSERT_MSG( numberOfBytesToRead <= MAXDWORD, "File read operations are limited to DWORD sizes" );
	if( numberOfBytesToRead > MAXDWORD )
	{
		return false;
	}

	HELIUM_ASSERT( buffer || numberOfBytesToRead == 0 );

	DWORD tempBytesRead;
	bool result = 1 == ::ReadFile( m_Handle, buffer, static_cast< DWORD >( numberOfBytesToRead ), &tempBytesRead, NULL );
	if ( result && numberOfBytesRead )
	{
		*numberOfBytesRead = tempBytesRead;
	}
	return result;
}

bool File::Write( const void* buffer, size_t numberOfBytesToWrite, size_t* numberOfBytesWritten )
{
	HELIUM_ASSERT_MSG( numberOfBytesToWrite <= MAXDWORD, "File write operations are limited to DWORD sizes" );
	if( numberOfBytesToWrite > MAXDWORD )
	{
		return false;
	}

	HELIUM_ASSERT( buffer || numberOfBytesToWrite == 0 );

	DWORD tempBytesWritten;
	bool result = 1 == ::WriteFile( m_Handle, buffer, static_cast< DWORD >( numberOfBytesToWrite ), &tempBytesWritten, NULL );
	if ( result && numberOfBytesWritten )
	{
		*numberOfBytesWritten = tempBytesWritten;
	}
	return result;
}

bool File::Flush()
{
	return 1 == ::FlushFileBuffers( m_Handle );
}

int64_t File::Seek( int64_t offset, SeekOrigin origin )
{
	LARGE_INTEGER moveDistance;
	moveDistance.QuadPart = offset;

	DWORD moveMethod =
		( origin == SeekOrigins::Current
		? FILE_CURRENT
		: ( origin == SeekOrigins::Begin ? FILE_BEGIN : FILE_END ) );

	LARGE_INTEGER filePointer;
	filePointer.QuadPart = 0;

	BOOL bResult = ::SetFilePointerEx( m_Handle, moveDistance, &filePointer, moveMethod );
	HELIUM_ASSERT( bResult );

	return ( bResult ? filePointer.QuadPart : -1 );
}

int64_t File::Tell() const
{
	LARGE_INTEGER moveDistance;
	moveDistance.QuadPart = 0;

	LARGE_INTEGER filePointer;
	filePointer.QuadPart = 0;

	BOOL bResult = ::SetFilePointerEx( m_Handle, moveDistance, &filePointer, FILE_CURRENT );
	HELIUM_ASSERT( bResult );

	return ( bResult ? filePointer.QuadPart : -1 );
}

int64_t File::GetSize() const
{
	LARGE_INTEGER fileSize;
	fileSize.QuadPart = 0;

	BOOL bResult = ::GetFileSizeEx( m_Handle, &fileSize );
	HELIUM_ASSERT( bResult );

	return ( bResult ? fileSize.QuadPart : -1 );
}

//
// File stats
//

Status::Status()
: m_Mode( 0 )
, m_Size( 0 )
, m_CreatedTime( 0 )
, m_ModifiedTime( 0 )
, m_AccessTime( 0 )
{

}

bool Status::Read( const char* path )
{
	HELIUM_TCHAR_TO_WIDE( path, convertedPath );

	WIN32_FILE_ATTRIBUTE_DATA fileStatus;
	memset( &fileStatus, 0, sizeof( fileStatus ) );
	bool result = ::GetFileAttributesEx( convertedPath, GetFileExInfoStandard, &fileStatus ) == TRUE;
	if ( result )
	{
		m_Size = ( (uint64_t)fileStatus.nFileSizeHigh << 32 ) | fileStatus.nFileSizeLow;
		m_CreatedTime = FromWindowsTime( fileStatus.ftCreationTime );
		m_ModifiedTime = FromWindowsTime( fileStatus.ftLastWriteTime );
		m_AccessTime = FromWindowsTime( fileStatus.ftLastAccessTime );

		FromWindowsAttributes( fileStatus.dwFileAttributes, m_Mode );
	}

	return result;
}

//
// Directory info
//

DirectoryEntry::DirectoryEntry( const std::string& name )
	: m_Name( name )
{
}

Directory::Directory( const std::string& path )
	: m_Path( path )
	, m_Handle( INVALID_HANDLE_VALUE )
{
}

Directory::~Directory()
{
	HELIUM_VERIFY( Close() );
}

bool Directory::IsOpen()
{
	return m_Handle != INVALID_HANDLE_VALUE;
}

bool Directory::FindFirst( DirectoryEntry& entry )
{
	Close();

	std::string path ( m_Path + "/*" );
	HELIUM_TCHAR_TO_WIDE( path.c_str(), convertedPath );

	WIN32_FIND_DATA foundFile;
	m_Handle = ::FindFirstFile( convertedPath, &foundFile );
	if ( m_Handle == INVALID_HANDLE_VALUE )
	{
		return false;
	}

	FromWindowsFindData( foundFile, entry );
	return true;
}

bool Directory::FindNext( DirectoryEntry& entry )
{
	WIN32_FIND_DATA foundFile;
	if ( !::FindNextFile( m_Handle, &foundFile ) )
	{
		return false;
	}

	FromWindowsFindData( foundFile, entry );
	return true;
}

bool Directory::Close()
{
	if ( IsOpen() && ::FindClose( m_Handle ) == 0 )
	{
		return false;
	}

	m_Handle = INVALID_HANDLE_VALUE;
	return true;
}

//
// File system ops
//

const char Helium::PathSeparator = '\\';

bool Helium::GetFullPath( const char* path, std::string& fullPath )
{
	HELIUM_TCHAR_TO_WIDE( path, convertedPath );
	DWORD fullPathNameCount = ::GetFullPathName( convertedPath, 0, NULL, NULL );
	wchar_t* fullPathName = (wchar_t*)alloca( sizeof(wchar_t) * fullPathNameCount );
	uint32_t result = ::GetFullPathName( convertedPath, fullPathNameCount, fullPathName, NULL );

	if ( result == 0 )
	{
		return false;
	}
	
	HELIUM_WIDE_TO_TCHAR( fullPathName, convertedFullPathName );
	fullPath = convertedFullPathName;
	return true;
}

bool Helium::IsAbsolute( const char* path )
{
	if ( path && path[0] != '\0' && path[1] != '\0' )
	{
		if ( path[ 1 ] == ':' )
		{
			return true;
		}

		if ( path[ 0 ] == '\\' && path[ 1 ] == '\\' )
		{
			return true;
		}
	}

	return false;
}

bool Helium::MakePath( const char* path )
{
	HELIUM_ASSERT(path[0] != '\0');

	std::vector< std::string > directories;
	SplitDirectories( path, directories );

	if ( directories.size() == 1 )
	{
		HELIUM_TCHAR_TO_WIDE( path, convertedPath );
		if ( !CreateDirectoryW( convertedPath, NULL ) )
		{
			return ::GetLastError() == ERROR_ALREADY_EXISTS;
		}
	}
	else
	{
		struct _stati64 statInfo;
		std::string currentDirectory;
		currentDirectory.reserve( MAX_PATH );
		currentDirectory = directories[ 0 ];
		std::vector< std::string >::const_iterator itr = directories.begin() + 1, end = directories.end();
		while( true )
		{
			HELIUM_TCHAR_TO_WIDE( currentDirectory.c_str(), convertedCurrentDirectory );

			if ( ( (*currentDirectory.rbegin()) != ':' ) && ( _wstat64( convertedCurrentDirectory, &statInfo ) != 0 ) )
			{
				if ( !CreateDirectoryW( convertedCurrentDirectory, NULL ) )
				{
					if ( ::GetLastError() != ERROR_ALREADY_EXISTS )
					{
						return false;
					}
				}
			}

			if( itr != end )
			{
				currentDirectory += std::string( "\\" ) + *itr;
				++itr;
			}
			else
			{
				break;
			}
		}
	}

	return true;
}

bool Helium::CreateDirectory( const char* path )
{
	HELIUM_TCHAR_TO_WIDE( path, convertedChars );
	return ::CreateDirectoryW( convertedChars, NULL ) == TRUE;
}

bool Helium::DeleteEmptyDirectory( const char* path )
{
	HELIUM_TCHAR_TO_WIDE( path, convertedChars );
	return ::RemoveDirectoryW( convertedChars ) == TRUE;
}

bool Helium::CopyFile( const char* source, const char* dest, bool overwrite )
{
	HELIUM_TCHAR_TO_WIDE( source, convertedSource );
	HELIUM_TCHAR_TO_WIDE( dest, convertedDest );
	return ( TRUE == ::CopyFileW( convertedSource, convertedDest, overwrite ? FALSE : TRUE ) );
}

bool Helium::MoveFile( const char* source, const char* dest )
{
	HELIUM_TCHAR_TO_WIDE( source, convertedSource );
	HELIUM_TCHAR_TO_WIDE( dest, convertedDest );
	return ( TRUE == ::MoveFileW( convertedSource, convertedDest ) );
}

bool Helium::DeleteFile( const char* path )
{
	HELIUM_TCHAR_TO_WIDE( path, convertedPath );
	return ( TRUE == ::DeleteFileW( convertedPath ) );
}