#include "Precompile.h"
#include "File.h"

#include "Platform/Assert.h"
#include "Platform/Encoding.h"
#include "Platform/Types.h"

#include <errno.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if HELIUM_OS_LINUX
# include <sys/sendfile.h>
#endif

#if HELIUM_OS_MAC
# include <copyfile.h>
#endif

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

bool File::Open( const char* filename, FileMode mode, bool truncate )
{
	int flags = 0;
	switch ( mode )
	{
	case FileModes::Read:
		flags = O_RDONLY;
		break;

	case FileModes::Write:
		flags = O_WRONLY;
		break;

	case FileModes::Both:
		flags = O_RDWR;
		break;
	}

	mode_t permissions = 0;
	if ( mode & FileModes::Write )
	{
		flags |= O_CREAT;
		permissions |= S_IRUSR | S_IWUSR;

		if ( truncate )
		{
			flags |= O_TRUNC;
		}
	}

	m_Handle = open( filename, flags, permissions);
	return m_Handle >= 0;
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

	if (numberOfBytesRead)
	{
		*numberOfBytesRead = numberOfBytesToRead;
	}
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

	if (numberOfBytesWritten)
	{
		*numberOfBytesWritten = numberOfBytesToWrite;
	}
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

bool Status::Read( const char* path )
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

DirectoryEntry::DirectoryEntry( const std::string& name )
	: m_Name( name )
{
}

Directory::Directory( const std::string& path )
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

const char Helium::PathSeparator = '/';

bool Helium::GetFullPath( const char* path, std::string& fullPath )
{
	HELIUM_ASSERT( sizeof( path ) > 0 );
	
	// unbelievably, posix doesn't seem to have a built-in path normalization function
	// that doesn't require the file to exist
	
	std::vector< std::string > directories;
	SplitDirectories( path, directories );

	if ( directories.empty() )
	{
		return false;
	}

	std::vector< std::string > normalizedDirs;
	
	for( std::vector< std::string >::const_iterator itr = directories.begin(), end = directories.end(); itr != end; ++itr )
	{
		if ( *itr == "." )
		{
			continue;
		}
		else if ( *itr == ".." )
		{
			normalizedDirs.pop_back();
		}
		else
		{
			normalizedDirs.push_back( *itr );
		}
	}

	fullPath.reserve( PATH_MAX );
	fullPath.clear();
	
	// if it's not absolute, start with CWD
	if ( path[ 0 ] != Helium::PathSeparator )
	{
		char cwd[ PATH_MAX ];
		char* result = getcwd( cwd, PATH_MAX );
		HELIUM_ASSERT( result ); // will fire when cwd isn't large enough
		fullPath = cwd;
	}
	
	for( std::vector< std::string >::const_iterator itr = normalizedDirs.begin(), end = normalizedDirs.end(); itr != end; ++itr )
	{
		fullPath += Helium::PathSeparator;
		fullPath += *itr;
	}

	return true;
}

bool Helium::IsAbsolute( const char* path )
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

bool Helium::MakePath( const char* path )
{
	HELIUM_ASSERT(path[0] != '\0');

	std::string fullPath;
	if ( !GetFullPath( path, fullPath ) )
	{
		return false;
	}

	std::vector< std::string > directories;
	SplitDirectories( fullPath, directories );

	if ( directories.empty() )
	{
		return false;
	}
	
	struct stat status;
	std::string currentDirectory;
	currentDirectory.reserve( PATH_MAX );
	currentDirectory = Helium::PathSeparator + directories[ 0 ];
	std::vector< std::string >::const_iterator itr = directories.begin() + 1, end = directories.end();
	while( true )
	{
		if ( stat( currentDirectory.c_str(), &status ) != 0 )
		{
			if ( mkdir( currentDirectory.c_str(), 0777 ) != 0 )
			{
				if ( errno != EEXIST )
				{
					return false;
				}
			}
		}

		if( itr != end )
		{
			currentDirectory += Helium::PathSeparator;
			currentDirectory += *itr;
			++itr;
		}
		else
		{
			break;
		}
	}

	return true;
}

bool Helium::CreateDirectory( const char* path )
{
	return 0 == mkdir( path, 0777 );
}

bool Helium::DeleteEmptyDirectory( const char* path )
{
	return 0 == rmdir( path );
}

bool Helium::CopyFile( const char* source, const char* dest, bool overwrite )
{
#if HELIUM_OS_LINUX
	struct stat status;
	if ( stat( source, &status ) != 0 )
	{
		return false;
	}

	if ( overwrite )
	{
		struct stat destStatus;
		if ( stat( dest, &destStatus ) == 0 )
		{
			if ( unlink( dest ) != 0 )
			{
				return false;
			}
		}
	}

	int input, output;

	if( (input = open(source, O_RDONLY)) == -1)
	{
		return false;
	}

	if( (output = open(dest, O_WRONLY | O_CREAT, 0666)) == -1)
	{
		close(input);
		return false;
	}

	bool result = sendfile(output, input, 0, status.st_size) != -1;

	close(input);
	close(output);

	return result;
#else
	return 0 == copyfile( source, dest, NULL, COPYFILE_STAT | COPYFILE_DATA );
#endif
}

bool Helium::MoveFile( const char* source, const char* dest )
{
	return 0 == rename( source, dest );
}

bool Helium::DeleteFile( const char* path )
{
	return 0 == unlink( path );
}
