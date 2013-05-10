#include "PlatformPch.h"
#include "Platform/File.h"

#include "Platform/Assert.h"
#include "Platform/Encoding.h"
#include "Platform/Types.h"

#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace Helium;

//
// File contents
//

File::File()
	: m_Handle( -1 )
{
}

File::~File()
{
	HELIUM_VERIFY( Close() );
}

bool File::IsOpen() const
{
	return m_Handle >= 0;
}

bool File::Open( const tchar_t* filename, FileMode mode, bool truncate )
{
	mode_t flags = 0;
	switch ( mode )
	{
	case FileModes::Read:
		flags = O_RDONLY;

	case FileModes::Write:
		flags = O_WRONLY;

	case FileModes::Read | FileModes::Write:
		flags = O_RDWR;
	}

	if ( mode & FileModes::Write )
	{
		flags |= O_CREAT;
	}

	if ( truncate )
	{
		flags |= O_TRUNC;
	}

	m_Handle = open( filename, flags );
	return m_Handle < 0;
}

bool File::Close()
{
	if ( IsOpen() && close( m_Handle ) != 0 )
	{
		return false;
	}

	m_Handle = -1;
	return true;
}

bool File::Read( void* buffer, size_t numberOfBytesToRead, size_t* numberOfBytesRead )
{
	HELIUM_ASSERT( buffer || numberOfBytesToRead == 0 );
	int result = read( m_Handle, buffer, numberOfBytesToRead );
	if ( result < 0 )
	{
		return false;
	}

	*numberOfBytesRead == numberOfBytesToRead;
	return true;
}

bool File::Write( const void* buffer, size_t numberOfBytesToWrite, size_t* numberOfBytesWritten )
{
	HELIUM_ASSERT( buffer || numberOfBytesToWrite == 0 );
	int result = write( m_Handle, buffer, numberOfBytesToWrite );
	if ( result < 0 )
	{
		return false;
	}

	*numberOfBytesWritten == numberOfBytesToWrite;
	return true;
}

bool File::Flush()
{
	return 0 == fsync( m_Handle );
}

int64_t File::Seek( int64_t offset, SeekOrigin origin )
{
	int whence =
		( origin == SeekOrigins::Current
		? SEEK_CUR
		: ( origin == SeekOrigins::Begin ? SEEK_SET : SEEK_END ) );

	return lseek( m_Handle, offset, whence );
}

int64_t File::Tell() const
{
	return lseek( m_Handle, 0, SEEK_CUR );
}

int64_t File::GetSize() const
{
	struct stat status;
	fstat( m_Handle, &status );
	return status.st_size;
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

bool Status::Read( const tchar_t* path )
{
	struct stat status;
	if ( 0 == stat( path, &status ) )
	{
		m_Mode = 0;
		if ( status.st_mode & S_IFDIR )
		{
			m_Mode |= StatusModes::Directory;
		}
		if ( status.st_mode & S_IFLNK )
		{
			m_Mode |= StatusModes::Link;
		}
		if ( status.st_mode & S_IFIFO )
		{
			m_Mode |= StatusModes::Pipe;
		}
		if ( m_Mode == 0x0 )
		{
			m_Mode |= StatusModes::Special;
		}
		if ( status.st_mode & S_IRUSR )
		{
			m_Mode |= StatusModes::Read;
		}
		if ( status.st_mode & S_IWUSR )
		{
			m_Mode |= StatusModes::Write;
		}
		if ( status.st_mode & S_IXUSR )
		{
			m_Mode |= StatusModes::Execute;
		}

		m_Size = status.st_size;
		m_CreatedTime = status.st_ctime;
		m_ModifiedTime = status.st_mtime;
		m_AccessTime = status.st_atime;
		return true;
	}

	return false;
}

//
// Directory info
//

DirectoryEntry::DirectoryEntry( const tstring& name )
	: m_Name( name )
{
}

Directory::Directory( const tstring& path )
	: m_Path( path )
	, m_Handle( NULL )
{
}

Directory::~Directory()
{
	HELIUM_VERIFY( Close() );
}

bool Directory::IsOpen()
{
	return m_Handle != NULL;
}

bool Directory::FindFirst( DirectoryEntry& entry )
{
	Close();

	m_Handle = opendir( m_Path.c_str() );
	if ( m_Handle )
	{
		return FindNext( entry );
	}

	return false;
}

bool Directory::FindNext( DirectoryEntry& entry )
{
	struct dirent* dirEntry = readdir( m_Handle );
	if ( dirEntry )
	{
		entry.m_Name = dirEntry->d_name;
		entry.m_Stat.Read( entry.m_Name.c_str() );
		return true;
	}
	return false;
}

bool Directory::Close()
{
	if ( IsOpen() && closedir( m_Handle ) != 0 )
	{
		return false;
	}

	m_Handle = NULL;
	return true;
}

//
// File system ops
//

const tchar_t Helium::PathSeparator = TXT('/');

void Helium::GetFullPath( const tchar_t* path, tstring& fullPath )
{
	char* p = realpath( path, NULL );
	fullPath = p;
	free( p );
}

bool Helium::IsAbsolute( const tchar_t* path )
{
	if ( path && path[0] != '\0' )
	{
		if ( path[ 0 ] == PathSeparator )
		{
			return true;
		}
	}

	return false;
}

static void SplitDirectories( const tstring& path, std::vector< tstring >& output )
{
	tstring::size_type start = 0; 
	tstring::size_type end = 0; 
	while ( ( end = path.find( Helium::PathSeparator, start ) ) != tstring::npos )
	{ 
		output.push_back( path.substr( start, end - start ) ); 
		start = end + 1;
	}
	output.push_back( path.substr( start ) ); 
}

bool Helium::MakePath( const tchar_t* path )
{
	std::vector< tstring > directories;
	SplitDirectories( path, directories );

	struct stat status;
	tstring currentDirectory;
	currentDirectory.reserve( PATH_MAX );
	currentDirectory = directories[ 0 ];
	for( std::vector< tstring >::const_iterator itr = directories.begin() + 1, end = directories.end(); itr != end; ++itr )
	{
		if ( !IsAbsolute( currentDirectory.c_str() ) && stat( currentDirectory.c_str(), &status ) != 0 )
		{
			if ( !mkdir( currentDirectory.c_str(), 0777 ) )
			{
				return false;
			}
		}

		currentDirectory += tstring( "/" ) + *itr;
	}

	return true;
}

bool Helium::Copy( const tchar_t* source, const tchar_t* dest, bool overwrite )
{
#define splice(a, b, c) splice(a, 0, b, 0, c, 0)
	int p[2];
	pipe(p);
	int out = open(dest, O_WRONLY);
	int in = open(source, O_RDONLY);
	while(splice(p[0], out, splice(in, p[1], 4096))>0);
#undef splice
}

bool Helium::Move( const tchar_t* source, const tchar_t* dest )
{
	if ( Copy( source, dest, true ) )
	{
		return Delete( source );
	}

	return false;
}

bool Helium::Delete( const tchar_t* path )
{
	return unlink( path ) != 0;
}