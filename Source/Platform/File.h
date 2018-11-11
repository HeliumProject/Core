#pragma once

#include "Platform/API.h"
#include "Platform/Types.h"
#include "Platform/Utility.h"

#if !HELIUM_OS_WIN
#include <sys/types.h>
#include <dirent.h>
#endif

#include <string>
#include <vector>

namespace Helium
{
	//
	// File contents
	//

	namespace SeekOrigins
	{
		/// Stream seek origin.
		enum SeekOrigin
		{
			Current,  ///< Seek relative to the current location.
			Begin,    ///< Seek relative to the beginning of the stream.
			End,      ///< Seek relative to the end of the stream.
		};
	};
	typedef SeekOrigins::SeekOrigin SeekOrigin;

	namespace FileModes
	{
		/// File access mode flags.
		enum FileMode
		{
			Read  = ( 1 << 0 ),  ///< Read access.
			Write = ( 1 << 1 ),  ///< Write access.
			Both  = Read | Write
		};
	};
	typedef FileModes::FileMode FileMode;

	class HELIUM_PLATFORM_API File
	{
	public:
		File();
		~File();

		bool IsOpen() const;
		bool Open( const char* filename, FileMode mode, bool truncate = true );
		bool Close();
		
		bool Read( void* buffer, size_t numberOfBytesToRead, size_t* numberOfBytesRead = NULL );
		bool Write( const void* buffer, size_t numberOfBytesToWrite, size_t* numberOfBytesWritten = NULL );
		bool Flush();

		int64_t Seek( int64_t offset, SeekOrigin origin );
		int64_t Tell() const;
		int64_t GetSize() const;

	private:
#ifdef HELIUM_OS_WIN
		typedef void* Handle;
#else
		typedef int Handle;
#endif
		Handle m_Handle;
	};

	//
	// File status
	//

	namespace StatusModes
	{
		enum Type
		{
			None             = 0,
			Directory        = 1 << 0,
			Link             = 1 << 1,
			Pipe             = 1 << 2,
			Special          = 1 << 3,

			Read             = 1 << 8,
			Write            = 1 << 9,
			Execute          = 1 << 10
		};
	}
	typedef StatusModes::Type StatusMode;

	class HELIUM_PLATFORM_API Status
	{
	public:
		Status();

		bool Read( const char* path );

		uint32_t     m_Mode;
		uint64_t     m_Size;
		uint64_t     m_CreatedTime;
		uint64_t     m_ModifiedTime;
		uint64_t     m_AccessTime;
	};

	//
	// Directory info
	//

	class HELIUM_PLATFORM_API DirectoryEntry
	{
	public:
		DirectoryEntry( const std::string& name = "" );

		std::string	m_Name;
		Status	m_Stat;
	};

	class HELIUM_PLATFORM_API Directory : NonCopyable
	{
	public:
		Directory( const std::string& path = "" );
		~Directory();

		bool IsOpen();
		bool FindFirst( DirectoryEntry& entry );
		bool FindNext( DirectoryEntry& entry );
		bool Close();

		inline const std::string& GetPath();
		inline void SetPath( const std::string& path );

	private:
		std::string	m_Path;

#if HELIUM_OS_WIN
		typedef void* Handle;
#else
		typedef DIR* Handle;
#endif
		Handle m_Handle;
	};

	//
	// File system operations
	//

	HELIUM_PLATFORM_API extern const char PathSeparator;
	HELIUM_PLATFORM_API bool GetFullPath( const char* path, std::string& fullPath );
	HELIUM_PLATFORM_API bool IsAbsolute( const char* path );
	HELIUM_PLATFORM_API void SplitDirectories( const std::string& path, std::vector< std::string >& output );

	HELIUM_PLATFORM_API bool MakePath( const char* path );
	HELIUM_PLATFORM_API bool CreateDirectory( const char* path );
	HELIUM_PLATFORM_API bool DeleteEmptyDirectory( const char* path );

	HELIUM_PLATFORM_API bool CopyFile( const char* source, const char* dest, bool overwrite );
	HELIUM_PLATFORM_API bool MoveFile( const char* source, const char* dest );
	HELIUM_PLATFORM_API bool DeleteFile( const char* path );
}

#include "Platform/File.inl"
