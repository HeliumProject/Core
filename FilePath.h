#pragma once

#include <set>
#include <vector>

#include "Platform/File.h"

#include "Foundation/API.h"
#include "Foundation/SmartPtr.h"
#include "Foundation/String.h"

namespace Helium
{
	const static char s_InternalPathSeparator = '/';

	class HELIUM_FOUNDATION_API FilePath
	{
	private:
		std::string m_Path;

		void Init( const char* path );

	public:
		static void Normalize( std::string& path );
		static void MakeNative( std::string& path );
		static void GuaranteeSeparator( std::string& path );

		static bool Exists( const std::string& path );
		static bool IsAbsolute( const char* path );
		static bool IsUnder( const std::string& location, const std::string& path );

	public:
		explicit FilePath( const std::string& path = TXT( "" ) );
		FilePath( const FilePath& path );

		const char* operator*() const;

		FilePath& operator=( const FilePath& rhs );
		bool operator==( const FilePath& rhs ) const;

		bool operator<( const FilePath& rhs ) const;

		FilePath operator+( const char* rhs ) const;
		FilePath operator+( const std::string& rhs ) const;
		FilePath operator+( const FilePath& rhs ) const;

		FilePath& operator+=( const char* rhs );
		FilePath& operator+=( const std::string& rhs );
		FilePath& operator+=( const Helium::FilePath& rhs );

		const std::string& Get() const;
		void Set( const std::string& path );
		void Set( const String& path );
		void Clear();

		void TrimToExisting();

		void Split( std::string& directory, std::string& filename ) const;
		void Split( std::string& directory, std::string& filename, std::string& extension ) const;

		std::string Basename() const;
		std::string Filename() const;
		std::string Directory() const;
		std::vector< std::string > DirectoryAsVector() const;

		std::string Extension() const;
		std::string FullExtension() const;
		void RemoveExtension();
		void RemoveFullExtension();
		void ReplaceExtension( const std::string& newExtension );
		void ReplaceFullExtension( const std::string& newExtension );
		bool HasExtension( const char* extension ) const;

		std::string Native() const;
		std::string Absolute() const;
		std::string Normalized() const;
		std::string Signature();

		bool Exists() const;
		bool IsAbsolute() const;
		bool IsUnder( const std::string& location ) const;
		bool IsFile() const;
		bool IsDirectory() const;
		bool Writable() const;
		bool Readable() const;

		bool MakePath() const;
		bool Create() const;
		bool Copy( const Helium::FilePath& target, bool overwrite = true ) const;
		bool Move( const Helium::FilePath& target ) const;
		bool Delete() const;

		std::string FileMD5() const;
		bool VerifyFileMD5( const std::string& hash ) const;

	public:

		Helium::FilePath GetAbsolutePath( const Helium::FilePath& basisPath ) const;
		Helium::FilePath GetRelativePath( const Helium::FilePath& basisPath ) const;

	public:

		size_t length() const;
		bool empty() const;
		const char* c_str() const;
		operator const char*() const
		{
			return c_str();
		}
		operator const std::string&() const
		{
			return m_Path;
		}

		static const FilePath NULL_FILE_PATH;
	};
}
