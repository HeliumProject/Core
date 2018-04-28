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
	public:
		FilePath();
		FilePath( const std::string& path );
		FilePath( const FilePath& path );

		FilePath& operator=( const FilePath& rhs );
		bool operator==( const FilePath& rhs ) const;
		bool operator!=( const FilePath& rhs ) const;
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

		size_t Length() const;
		bool Empty() const;
		const char* Data() const;

		void Split( FilePath& directory, FilePath& filename ) const;
		void Split( FilePath& directory, FilePath& filename, std::string& extension ) const;

		// relatives
		FilePath Directory() const;
		FilePath Parent() const;

		// components
		void Directories( std::vector< std::string >& directories ) const;
		FilePath Filename() const;
		std::string Basename() const;
		std::string Extension() const;
		std::string FullExtension() const;
		void RemoveExtension();
		void RemoveFullExtension();
		void ReplaceExtension( const std::string& newExtension );
		void ReplaceFullExtension( const std::string& newExtension );
		bool HasExtension( const char* extension ) const;

		// munging
		FilePath Native() const;
		FilePath Absolute() const;
		FilePath Normalized() const;

		// utility
		bool Exists() const;
		bool IsAbsolute() const;
		bool IsUnder( const FilePath& location ) const;
		bool IsFile() const;
		bool IsDirectory() const;
		bool Writable() const;
		bool Readable() const;

		// file
		void TrimToExisting();
		bool MakePath() const;
		bool Create() const;
		bool Copy( const Helium::FilePath& target, bool overwrite = true ) const;
		bool Move( const Helium::FilePath& target ) const;
		bool Delete() const;

		// meta
		std::string MD5() const;
		std::string FileMD5() const;
		bool VerifyFileMD5( const std::string& hash ) const;

		// relpath
		Helium::FilePath GetAbsolutePath( const Helium::FilePath& basisPath ) const;
		Helium::FilePath GetRelativePath( const Helium::FilePath& basisPath ) const;

		static void Normalize( FilePath& path );
		static void MakeNative( FilePath& path );
		static void GuaranteeSeparator( FilePath& path );

		static bool Exists( const FilePath& path );
		static bool IsAbsolute( const FilePath& path );
		static bool IsUnder( const FilePath& location, const FilePath& path );

	private:
		void Init( const char* path );

		std::string m_Path;
	};
}
