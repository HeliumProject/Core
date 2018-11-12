#include "Precompile.h"
#include "FilePath.h"

#include "Platform/Exception.h"
#include "Platform/File.h"
#include "Platform/Types.h"

#include "Foundation/String.h"
#include "Foundation/Crc32.h"
#include "Foundation/MD5.h"
	
#include <algorithm>
#include <sstream>

// TODO: change this from being a string-based class to use Name so its more efficient for pass-by-value -geoff
// TODO: remove all API's that obfustcate excessive statting (like IsFile, IsDirectory) -geoff

using namespace Helium;

FilePath::FilePath()
{
}

FilePath::FilePath( const std::string& path )
{
	FilePath::Init( path.c_str() );
}

FilePath::FilePath( const FilePath& path )
{
	FilePath::Init( path.Data() );
}

FilePath& FilePath::operator=( const FilePath& rhs )
{
	FilePath::Init( rhs.Data() );
	return *this;
}

bool FilePath::operator==( const FilePath& rhs ) const
{
#if HELIUM_OS_LINUX
	return CompareString( Data(), rhs.Data() ) == 0;
#else
	return CaseInsensitiveCompareString( Data(), rhs.Data() ) == 0;
#endif
}

bool FilePath::operator!=( const FilePath& rhs ) const
{
#if HELIUM_OS_LINUX
	return CompareString( Data(), rhs.Data() ) != 0;
#else
	return CaseInsensitiveCompareString( Data(), rhs.Data() ) != 0;
#endif
}

bool FilePath::operator<( const FilePath& rhs ) const
{
#if HELIUM_OS_LINUX
	return CompareString( Data(), rhs.Data() ) < 0;
#else
	return CaseInsensitiveCompareString( Data(), rhs.Data() ) < 0;
#endif
}

Helium::FilePath FilePath::operator+( const char* rhs ) const
{
	return FilePath( std::string( Get() + rhs ) );
}

Helium::FilePath FilePath::operator+( const std::string& rhs ) const
{
	return Helium::FilePath( Get() + rhs );
}

Helium::FilePath FilePath::operator+( const Helium::FilePath& rhs ) const
{
	// you shouldn't use this on an absolute path
	HELIUM_ASSERT( !rhs.IsAbsolute() );
	return rhs.GetAbsolutePath( *this );
}

Helium::FilePath& FilePath::operator+=( const char* rhs )
{
	Set( Get() + rhs );
	return *this;
}

Helium::FilePath& FilePath::operator+=( const std::string& rhs )
{
	Set( Get() + rhs );
	return *this;
}

Helium::FilePath& FilePath::operator+=( const Helium::FilePath& rhs )
{
	// you shouldn't use this on an absolute path
	HELIUM_ASSERT( !rhs.IsAbsolute() );
	*this = rhs.GetAbsolutePath( *this );
	return *this;
}

const std::string& FilePath::Get() const
{
	return m_Path;
}

void FilePath::Set( const std::string& path )
{
	Init( path.c_str() );
}

void FilePath::Set( const String& path )
{
	Init( path.GetData() );
}

void FilePath::Clear()
{
	Set( "" );
}

size_t FilePath::Length() const
{
	return m_Path.length();
}

bool FilePath::Empty() const
{
	return m_Path.empty();
}

const char* FilePath::Data() const
{
	return m_Path.data();
}

void FilePath::Split( FilePath& directory, FilePath& filename ) const
{
	directory = Directory();
	filename = Filename();
}

void FilePath::Split( FilePath& directory, FilePath& filename, std::string& extension ) const
{
	Split( directory, filename );
	extension = Extension();
}

FilePath FilePath::Directory() const
{
	size_t pos = m_Path.rfind( s_InternalPathSeparator );
	if (pos != std::string::npos)
	{
		return m_Path.substr( 0, pos + 1 );
	}

	return FilePath();
}

FilePath FilePath::Parent() const
{
	std::string path ( m_Path );

	if (!path.empty())
	{
		if (*path.rbegin() == s_InternalPathSeparator)
		{
			path.resize( path.size() - 1 );
		}
	}

	if (!path.empty())
	{
		// strip the final slash and subsequent chars
		size_t pos = path.rfind( s_InternalPathSeparator );
		if (pos != std::string::npos)
		{
			return path.substr( 0, pos + 1 );
		}

#if HELIUM_OS_WIN
		if (path.length() == 2)
		{
			if (path[1] == ':')
			{
				// root dir path
				return FilePath();
			}
		}
#endif
	}

	return FilePath( path );
}

void FilePath::Directories( std::vector< std::string >& directories ) const
{
	std::string dir( Directory().Get() );
	std::istringstream iss( Directory().Get() );
	do
	{
		std::string tmp;
		std::getline( iss, tmp, s_InternalPathSeparator );
		if (!iss)
		{
			break;
		}
		directories.push_back( tmp );
	} while (iss);
}

FilePath FilePath::Filename() const
{
	size_t pos = m_Path.rfind( s_InternalPathSeparator );
	if (pos != std::string::npos)
	{
		return m_Path.substr( pos + 1 );
	}

	return FilePath( m_Path );
}

std::string FilePath::Basename() const
{
	std::string basename = Filename().Get();

	size_t pos = basename.rfind( '.' );

	if ( pos != std::string::npos )
	{
		return basename.substr( 0, pos );
	}

	return basename;
}

std::string FilePath::Extension() const
{
	std::string filename = Filename().Get();

	size_t pos = filename.rfind( '.' );
	if ( pos != std::string::npos )
	{
		return filename.substr( pos + 1 );
	}

	return std::string();
}

std::string FilePath::FullExtension() const
{
	std::string filename = Filename().Get();

	size_t pos = filename.find_first_of( '.' );
	if ( pos != std::string::npos )
	{
		return filename.substr( pos + 1 );
	}

	return std::string();
}

void FilePath::RemoveExtension()
{
	size_t slash = m_Path.find_last_of( s_InternalPathSeparator );
	size_t pos = m_Path.find_last_of( '.', slash == std::string::npos ? 0 : slash );
	if ( pos != std::string::npos )
	{
		m_Path.erase( pos );
	}
}

void FilePath::RemoveFullExtension()
{
	size_t slash = m_Path.find_last_of( s_InternalPathSeparator );
	size_t pos = m_Path.find_first_of( '.', slash == std::string::npos ? 0 : slash );
	if ( pos != std::string::npos )
	{
		m_Path.erase( pos );
	}
}

void FilePath::ReplaceExtension( const std::string& newExtension )
{
	size_t slash = m_Path.find_last_of( s_InternalPathSeparator );
	size_t offset = m_Path.rfind( '.' );
	if ( offset != std::string::npos && ( offset > ( slash != std::string::npos ? slash : 0 ) ) )
	{
		m_Path.replace( offset + 1, newExtension.length(), newExtension );
	}
	else
	{
		m_Path += '.' + newExtension;
	}
}

void FilePath::ReplaceFullExtension( const std::string& newExtension )
{
	size_t slash = m_Path.find_last_of( s_InternalPathSeparator );
	size_t offset = m_Path.find_first_of( '.', slash == std::string::npos ? 0 : slash );
	if ( offset != std::string::npos )
	{
		m_Path.replace( offset + 1, newExtension.length(), newExtension );
	}
	else
	{
		m_Path += '.' + newExtension;
	}
}

bool FilePath::HasExtension( const char* extension ) const
{
	size_t len = StringLength( extension );

	if ( m_Path.length() < len )
	{
		return false;
	}

#if HELIUM_OS_LINUX
	return CompareString( Data() + ( m_Path.length() - len ), extension ) == 0;
#else
	return CaseInsensitiveCompareString( Data() + ( m_Path.length() - len ), extension ) == 0;
#endif
}

FilePath FilePath::Native() const
{
	FilePath native ( m_Path );
	FilePath::MakeNative( native );    
	return native;
}

FilePath FilePath::Absolute() const
{
	std::string full;
	Helium::GetFullPath( Data(), full );
	return FilePath( full );
}

FilePath FilePath::Normalized() const
{
	FilePath normalized( m_Path );
	FilePath::Normalize( normalized );
	return normalized;
}

bool FilePath::Exists() const
{
	return FilePath::Exists( *this );
}

bool FilePath::IsAbsolute() const
{
	return FilePath::IsAbsolute( *this );
}

bool FilePath::IsUnder( const FilePath& location ) const
{
	return FilePath::IsUnder( location, *this );
}

bool FilePath::IsFile() const
{
	if ( *(m_Path.rbegin()) == s_InternalPathSeparator )
	{
		return false;
	}

	Status stat;
	if ( !stat.Read( Data() ) )
	{
		return false;
	}

	return !( stat.m_Mode & Helium::StatusModes::Directory );
}

bool FilePath::IsDirectory() const
{
	if ( *(m_Path.rbegin()) == s_InternalPathSeparator )
	{
		return true;
	}

	Status stat;
	if ( !stat.Read( Data() ) )
	{
		return false;
	}

	return ( stat.m_Mode & Helium::StatusModes::Directory );
}

bool FilePath::Writable() const
{
	Status stat;
	if ( stat.Read( Data() ) )
	{
		return true;
	}

	return ( stat.m_Mode & Helium::StatusModes::Write ) == Helium::StatusModes::Write;
}

bool FilePath::Readable() const
{
	Status stat;
	if ( stat.Read( Data() ) )
	{
		return false;
	}

	return ( stat.m_Mode & Helium::StatusModes::Read ) == Helium::StatusModes::Read;
}

void FilePath::TrimToExisting()
{
	if ( !Exists() )
	{
		*this = Directory();
	}

	while ( !m_Path.empty() && !Exists() )
	{
		std::vector< std::string > directories;
		Directories( directories );
		std::string newDir;
		for( std::vector< std::string >::const_iterator itr = directories.begin(), end = directories.end(); itr != end && itr != end - 1; ++itr )
		{
			newDir += *itr + s_InternalPathSeparator;
		}

		Set( newDir );
	}
}

bool FilePath::MakePath() const
{
	FilePath dir = Directory();
	FilePath::MakeNative( dir );
	return Helium::MakePath( dir.Data() ); // expects native seps
}

bool FilePath::Create() const
{
	if ( !MakePath() )
	{
		return false;
	}

	File f;
	if ( !f.Open( Data(), FileModes::Write, true ) )
	{
		return false;
	}
	f.Close();

	return true;
}

bool FilePath::Copy( const Helium::FilePath& target, bool overwrite ) const
{
	return Helium::CopyFile( Data(), target.Data(), overwrite );
}

bool FilePath::Move( const Helium::FilePath& target ) const 
{
	return Helium::MoveFile( Data(), target.Data() );
}

bool FilePath::Delete() const
{
	return Helium::DeleteFile( Data() );
}

std::string FilePath::MD5() const
{
	FilePath temp ( m_Path );
	Normalize( temp );
	return Helium::MD5( temp.Get() );
}

std::string FilePath::FileMD5() const
{
	return Helium::FileMD5( Data() );
}

bool FilePath::VerifyFileMD5( const std::string& hash ) const
{
	return FileMD5().compare( hash ) == 0;
}

Helium::FilePath FilePath::GetAbsolutePath( const Helium::FilePath& basisPath ) const
{
	HELIUM_ASSERT( !IsAbsolute() ); // shouldn't call this on an already-absolute path

	std::string newPathtstring;
	Helium::GetFullPath( ( basisPath.Directory() + m_Path ).Get().c_str(), newPathtstring );
	return Helium::FilePath( newPathtstring );
}

Helium::FilePath FilePath::GetRelativePath( const Helium::FilePath& basisPath ) const
{
	std::vector< std::string > targetDirectories;
	this->Directories( targetDirectories );

	std::vector< std::string > baseDirectories;
	basisPath.Directories( baseDirectories );

	size_t i = 0;
	while( targetDirectories.size() > i && baseDirectories.size() > i && ( targetDirectories[ i ] == baseDirectories[ i ] ) )
	{
		++i;
	}

	if ( i == 0 )
	{
		return *this;
	}

	std::string newPathtstring;
	for ( size_t j = 0; j < ( baseDirectories.size() - i ); ++j )
	{
		newPathtstring += std::string( ".." ) + s_InternalPathSeparator;
	}

	for ( size_t j = i; j < targetDirectories.size(); ++j )
	{
		newPathtstring += targetDirectories[ j ] + s_InternalPathSeparator;
	}

	newPathtstring += Filename().Get();
	return Helium::FilePath( newPathtstring );
}

void FilePath::Normalize( FilePath& path )
{
	std::transform( path.m_Path.begin(), path.m_Path.end(), path.m_Path.begin(), tolower); 
	std::replace( path.m_Path.begin(), path.m_Path.end(), Helium::PathSeparator, s_InternalPathSeparator );
}

void FilePath::MakeNative( FilePath& path )
{
	std::replace( path.m_Path.begin(), path.m_Path.end(), s_InternalPathSeparator, Helium::PathSeparator );
}

void FilePath::GuaranteeSeparator( FilePath& path )
{
	if ( !path.m_Path.empty() && *path.m_Path.rbegin() != s_InternalPathSeparator )
	{
		path.m_Path += s_InternalPathSeparator;
	}
}

bool FilePath::Exists( const FilePath& path )
{
	Status stat;
	return stat.Read( path.Data() );
}

bool FilePath::IsAbsolute( const FilePath& path )
{
	return Helium::IsAbsolute( path.Data() );
}

bool FilePath::IsUnder( const FilePath& location, const FilePath& path )
{
#if HELIUM_OS_LINUX
	return CompareString( location.Data(), path.Data(), location.m_Path.length() ) == 0;
#else
	return CaseInsensitiveCompareString( location.Data(), path.Data(), location.m_Path.length() ) == 0;
#endif
}

void FilePath::Init( const char* path )
{
	m_Path = path;
	std::replace( m_Path.begin(), m_Path.end(), Helium::PathSeparator, s_InternalPathSeparator );
}
