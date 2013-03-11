#include "PersistPch.h"
#include "Persist/Archive.h"

#include "Platform/Locks.h"
#include "Platform/Process.h"
#include "Platform/Exception.h"

#include "Foundation/Log.h"
#include "Foundation/Profile.h"

#include "Reflect/Object.h"
#include "Reflect/DataDeduction.h"
#include "Reflect/Registry.h"

#include "Persist/ArchiveBinary.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <memory>

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;

Archive::Archive()
	: m_Progress( 0 )
	, m_Abort( false )
	, m_Mode( ArchiveModes::Read )
{
}

Archive::Archive( const FilePath& path )
	: m_Path( path )
	, m_Progress( 0 )
	, m_Abort( false )
	, m_Mode( ArchiveModes::Read )
{
	HELIUM_ASSERT( !m_Path.empty() );
}

Archive::~Archive()
{
}

void Archive::Put( Object* object )
{
	m_Object = object;
}

ObjectPtr Archive::Get( const Class* searchClass )
{
	PERSIST_SCOPE_TIMER( ( "%s", m_Path.c_str() ) );

	ObjectPtr object;
	Get( object );

	if ( searchClass == NULL )
	{
		searchClass = Reflect::GetClass< Object >();
	}

	if ( object->IsClass( searchClass ) )
	{
		return object;
	}
	else
	{
		return NULL;
	}
}

void Archive::Get( ObjectPtr& object )
{
	PERSIST_SCOPE_TIMER( ( "%s", m_Path.c_str() ) );

	Log::Debug( TXT( "Parsing '%s'\n" ), m_Path.c_str() );

	if ( Helium::IsDebuggerPresent() )
	{
		Open();
		Read();
		Close(); 
	}
	else
	{
		try
		{
			Open();

			try
			{
				Read();
			}
			catch (...)
			{
				Close();
				throw;
			}

			Close(); 
		}
		catch (Helium::Exception& ex)
		{
			tstringstream str;
			str << "While reading '" << m_Path.c_str() << "': " << ex.Get();
			ex.Set( str.str() );
			throw;
		}
	}

	object = m_Object;
}

ArchivePtr Persist::GetArchive( const FilePath& path, ArchiveType archiveType )
{
	switch ( archiveType )
	{
	case ArchiveTypes::Auto:
		{
			if ( CaseInsensitiveCompareString( path.Extension().c_str(), TXT( "reflect" ) ) )
			{
				return new ArchiveBinary( path );
			}
			break;
		}

	case ArchiveTypes::Binary:
		return new ArchiveBinary( path );

	default:
		HELIUM_ASSERT( false );
		break;
	}

	throw Reflect::StreamException( TXT( "Unknown archive type" ) );
}

bool Persist::ToArchive( const FilePath& path, ObjectPtr object, ArchiveType archiveType, tstring* error )
{
	HELIUM_ASSERT( !path.empty() );
	PERSIST_SCOPE_TIMER( ( "%s", path.c_str() ) );

	path.MakePath();

	// build a path to a unique file for this process
	FilePath safetyPath( path.Directory() + Helium::GetProcessString() );
	safetyPath.ReplaceExtension( path.Extension() );

	ArchivePtr archive = GetArchive( safetyPath, archiveType );
	archive->Put( object );

	// generate the file to the safety location
	if ( Helium::IsDebuggerPresent() )
	{
		archive->Open( true );
		archive->Write();
		archive->Close();
	}
	else
	{
		bool open = false;

		try
		{
			archive->Open( true );
			open = true;
			archive->Write();
			archive->Close(); 
		}
		catch ( Helium::Exception& ex )
		{
			tstringstream str;
			str << "While writing '" << path.c_str() << "': " << ex.Get();

			if ( error )
			{
				*error = str.str();
			}

			if ( open )
			{
				archive->Close();
			}

			safetyPath.Delete();
			return false;
		}
	}

	try
	{
		// delete the destination file
		path.Delete();

		// move the written file to the destination location
		safetyPath.Move( path );
	}
	catch ( Helium::Exception& ex )
	{
		tstringstream str;
		str << "While moving '" << safetyPath.c_str() << "' to '" << path.c_str() << "': " << ex.Get();

		if ( error )
		{
			*error = str.str();
		}

		safetyPath.Delete();
		return false;
	}

	return true;
}
