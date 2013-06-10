#include "FoundationPch.h"
#include "FilePath.h"

#include "Platform/Exception.h"
#include "Platform/File.h"
#include "Platform/Types.h"

#include "Foundation/String.h"
#include "Foundation/Crc32.h"
#include "Foundation/MD5.h"
	
#include <algorithm>
#include <sstream>

using namespace Helium;

const FilePath FilePath::NULL_FILE_PATH;

void FilePath::Init( const char* path )
{
	m_Path = path;

	std::replace( m_Path.begin(), m_Path.end(), Helium::PathSeparator, s_InternalPathSeparator );
}

FilePath::FilePath( const std::string& path )
{
	Init( path.c_str() );
}

FilePath::FilePath( const FilePath& path )
{
	Init( path.m_Path.c_str() );
}

const char* FilePath::operator*() const
{
	static const char emptyString[] = { TXT( '\0' ) };

	const char* pString = m_Path.c_str();

	return ( pString ? pString : emptyString );
}

FilePath& FilePath::operator=( const FilePath& rhs )
{
	Init( rhs.m_Path.c_str() );
	return *this;
}

bool FilePath::operator==( const FilePath& rhs ) const
{
	return CaseInsensitiveCompareString( m_Path.c_str(), rhs.m_Path.c_str() ) == 0;
}

bool FilePath::operator<( const FilePath& rhs ) const
{
	return CaseInsensitiveCompareString( m_Path.c_str(), rhs.m_Path.c_str() ) < 0;
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

void FilePath::Normalize( std::string& path )
{
	std::transform( path.begin(), path.end(), path.begin(), tolower); 
	std::replace( path.begin(), path.end(), Helium::PathSeparator, s_InternalPathSeparator );
}

void FilePath::MakeNative( std::string& path )
{
	std::replace( path.begin(), path.end(), s_InternalPathSeparator, Helium::PathSeparator );
}

void FilePath::GuaranteeSeparator( std::string& path )
{
	if ( !path.empty() && *path.rbegin() != s_InternalPathSeparator )
	{
		path += s_InternalPathSeparator;
	}
}

bool FilePath::Exists( const std::string& path )
{
	Status stat;
	return stat.Read( path.c_str() );
}

bool FilePath::IsAbsolute( const char* path )
{
	return Helium::IsAbsolute( path );
}

bool FilePath::IsUnder( const std::string& location, const std::string& path )
{
	return CaseInsensitiveCompareString( location.c_str(), path.c_str(), location.length() ) == 0;
}

bool FilePath::IsFile() const
{
	if ( *(m_Path.rbegin()) == s_InternalPathSeparator )
	{
		return false;
	}

	Status stat;
	if ( !stat.Read( m_Path.c_str() ) )
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
	if ( !stat.Read( m_Path.c_str() ) )
	{
		return false;
	}

	return ( stat.m_Mode & Helium::StatusModes::Directory );
}

bool FilePath::Writable() const
{
	Status stat;
	if ( stat.Read( m_Path.c_str() ) )
	{
		return true;
	}

	return ( stat.m_Mode & Helium::StatusModes::Write ) == Helium::StatusModes::Write;
}

bool FilePath::Readable() const
{
	Status stat;
	if ( stat.Read( m_Path.c_str() ) )
	{
		return false;
	}

	return ( stat.m_Mode & Helium::StatusModes::Read ) == Helium::StatusModes::Read;
}

bool FilePath::MakePath() const
{
#pragma TODO( "FIXME: This seems excessive, but Helium::MakePath expects native separators" )
	std::string dir = Directory();
	FilePath::MakeNative( dir );
	return Helium::MakePath( dir.c_str() );
}

bool FilePath::Create() const
{
	if ( !MakePath() )
	{
		return false;
	}

	File f;
	if ( !f.Open( m_Path.c_str(), FileModes::Write, true ) )
	{
		return false;
	}
	f.Close();
	
	return true;
}

bool FilePath::Copy( const Helium::FilePath& target, bool overwrite ) const
{
	return Helium::Copy( m_Path.c_str(), target.m_Path.c_str(), overwrite );
}

bool FilePath::Move( const Helium::FilePath& target ) const 
{
	return Helium::Move( m_Path.c_str(), target.m_Path.c_str() );
}

bool FilePath::Delete() const
{
	return Helium::Delete( m_Path.c_str() );
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
	Set( TXT( "" ) );
}

void FilePath::TrimToExisting()
{
	if ( !Exists() )
	{
		Set( Directory() );
	}

	while ( !m_Path.empty() && !Exists() )
	{
		std::vector< std::string > directories = DirectoryAsVector();
		std::string newDir;
		for( std::vector< std::string >::const_iterator itr = directories.begin(), end = directories.end(); itr != end && itr != end - 1; ++itr )
		{
			newDir += *itr + s_InternalPathSeparator;
		}

		Set( newDir );
	}
}

void FilePath::Split( std::string& directory, std::string& filename ) const
{
	directory = Directory();
	filename = Filename();
}

void FilePath::Split( std::string& directory, std::string& filename, std::string& extension ) const
{
	Split( directory, filename );
	extension = Extension();
}

std::string FilePath::Basename() const
{
	std::string basename = Filename();
	size_t pos = basename.rfind( TXT( '.' ) );

	if ( pos != std::string::npos )
	{
		return basename.substr( 0, pos );
	}

	return basename;
}

std::string FilePath::Filename() const
{
	size_t pos = m_Path.rfind( s_InternalPathSeparator );
	if ( pos != std::string::npos )
	{
		return m_Path.substr( pos + 1 );
	}

	return m_Path;
}

std::string FilePath::Directory() const
{
	size_t pos = m_Path.rfind( s_InternalPathSeparator );
	if ( pos != std::string::npos )
	{
		return m_Path.substr( 0, pos + 1 );
	}

	return TXT( "" );
}

std::vector< std::string > FilePath::DirectoryAsVector() const
{
	std::istringstream iss( Directory() );
	std::vector< std::string > out;
	do
	{ 
		std::string tmp;
		std::getline( iss, tmp, s_InternalPathSeparator );
		if ( !iss )
		{
			break;
		}
		out.push_back( tmp ); 
	} while( iss );

	return out;
}

std::string FilePath::Extension() const
{
	std::string filename = Filename();
	size_t pos = filename.rfind( TXT( '.' ) );
	if ( pos != std::string::npos )
	{
		return filename.substr( pos + 1 );
	}

	return TXT( "" );
}

std::string FilePath::FullExtension() const
{
	std::string filename = Filename();
	size_t pos = filename.find_first_of( TXT( '.' ) );
	if ( pos != std::string::npos )
	{
		return filename.substr( pos + 1 );
	}

	return TXT( "" );
}

void FilePath::RemoveExtension()
{
	size_t slash = m_Path.find_last_of( s_InternalPathSeparator );
	size_t pos = m_Path.find_last_of( TXT( '.' ), slash == std::string::npos ? 0 : slash );
	if ( pos != std::string::npos )
	{
		m_Path.erase( pos );
	}
}

void FilePath::RemoveFullExtension()
{
	size_t slash = m_Path.find_last_of( s_InternalPathSeparator );
	size_t pos = m_Path.find_first_of( TXT( '.' ), slash == std::string::npos ? 0 : slash );
	if ( pos != std::string::npos )
	{
		m_Path.erase( pos );
	}
}

void FilePath::ReplaceExtension( const std::string& newExtension )
{
	size_t slash = m_Path.find_last_of( s_InternalPathSeparator );
	size_t offset = m_Path.rfind( TXT( '.' ) );
	if ( offset != std::string::npos && ( offset > ( slash != std::string::npos ? slash : 0 ) ) )
	{
		m_Path.replace( offset + 1, newExtension.length(), newExtension );
	}
	else
	{
		m_Path += TXT( '.' ) + newExtension;
	}
}

void FilePath::ReplaceFullExtension( const std::string& newExtension )
{
	size_t slash = m_Path.find_last_of( s_InternalPathSeparator );
	size_t offset = m_Path.find_first_of( TXT( '.' ), slash == std::string::npos ? 0 : slash );
	if ( offset != std::string::npos )
	{
		m_Path.replace( offset + 1, newExtension.length(), newExtension );
	}
	else
	{
		m_Path += TXT( '.' ) + newExtension;
	}
}

bool FilePath::HasExtension( const char* extension ) const
{
	size_t len = StringLength( extension );

	if ( m_Path.length() < len )
	{
		return false;
	}

	return CaseInsensitiveCompareString( m_Path.c_str() + ( m_Path.length() - len ), extension ) == 0;
}

std::string FilePath::Native() const
{
	std::string native = m_Path;
	FilePath::MakeNative( native );    
	return native;
}

std::string FilePath::Absolute() const
{
	std::string full;
	Helium::GetFullPath( m_Path.c_str(), full );
	return full;
}

std::string FilePath::Normalized() const
{
	std::string normalized = m_Path;
	FilePath::Normalize( normalized );
	return normalized;
}

std::string FilePath::Signature()
{
	std::string temp = m_Path;
	Normalize( temp );
	return Helium::MD5( temp );
}

Helium::FilePath FilePath::GetAbsolutePath( const Helium::FilePath& basisPath ) const
{
	HELIUM_ASSERT( !IsAbsolute() ); // shouldn't call this on an already-absolute path

	std::string newPathtstring;
	Helium::GetFullPath( std::string( basisPath.Directory() + m_Path ).c_str(), newPathtstring );
	return Helium::FilePath( newPathtstring );
}

Helium::FilePath FilePath::GetRelativePath( const Helium::FilePath& basisPath ) const
{
	std::vector< std::string > targetDirectories = this->DirectoryAsVector();
	std::vector< std::string > baseDirectories = basisPath.DirectoryAsVector();

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
		newPathtstring += std::string( TXT( ".." ) ) + s_InternalPathSeparator;
	}

	for ( size_t j = i; j < targetDirectories.size(); ++j )
	{
		newPathtstring += targetDirectories[ j ] + s_InternalPathSeparator;
	}

	newPathtstring += Filename();
	return Helium::FilePath( newPathtstring );
}

bool FilePath::Exists() const
{
	return FilePath::Exists( m_Path );
}

bool FilePath::IsAbsolute() const
{
	return FilePath::IsAbsolute( m_Path.c_str() );
}

bool FilePath::IsUnder( const std::string& location ) const
{
	return FilePath::IsUnder( location, m_Path );
}

size_t FilePath::length() const
{
	return m_Path.length();
}

bool FilePath::empty() const
{
	return m_Path.empty();
}

const char* FilePath::c_str() const
{
	return m_Path.c_str();
}

std::string FilePath::FileMD5() const
{
	return Helium::FileMD5( m_Path.c_str() );
}

bool FilePath::VerifyFileMD5( const std::string& hash ) const
{
	return FileMD5().compare( hash ) == 0;
}
