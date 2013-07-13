#include "PersistPch.h"
#include "Persist/Archive.h"

#include "Platform/Locks.h"
#include "Platform/Process.h"
#include "Platform/Exception.h"

#include "Foundation/Log.h"
#include "Foundation/Profile.h"

#include "Reflect/Object.h"
#include "Reflect/TranslatorDeduction.h"
#include "Reflect/Registry.h"

#include "Persist/ArchiveJson.h"
#include "Persist/ArchiveMessagePack.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <memory>

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;

const char* Persist::ArchiveExtensions[] =
{
	"bson",
	"json",
	"msgpack"
};

Archive::Archive()
	: m_Progress( 0 )
	, m_Abort( false )
	, m_Flags( 0x0 )
{
}

Archive::Archive( const FilePath& path )
	: m_Path( path )
	, m_Progress( 0 )
	, m_Abort( false )
	, m_Flags( 0x0 )
{
	HELIUM_ASSERT( !m_Path.empty() );
}

Archive::~Archive()
{
}

ArchiveWriter::ArchiveWriter( ObjectIdentifier* identifier )
	: m_Identifier( identifier )
{

}

ArchiveWriter::ArchiveWriter( const FilePath& filePath, ObjectIdentifier* identifier )
	: Archive( filePath )
	, m_Identifier( identifier )
{
}

ArchiveMode ArchiveWriter::GetMode() const
{
	return ArchiveModes::Write;
}

bool ArchiveWriter::Identify( Object* object, Name& identity )
{
	if ( !m_Identifier || !m_Identifier->Identify( object, identity ))
	{
		size_t index = Invalid< size_t >();
		for ( DynamicArray< ObjectPtr >::ConstIterator itr = m_Objects.Begin(), end = m_Objects.End(); itr != end; ++itr )
		{
			if ( itr->Ptr() == object )
			{
				index = m_Objects.GetIndex( itr );
				break;
			}
		}

		if ( index == Invalid< size_t >() )
		{
			index = m_Objects.GetSize();

			// this will cause it to be written after the current object-in-progress (see Write)
			m_Objects.Push( object );
		}

		String str;
		str.Format( "%d", index );
		identity.Set( str );
	}

	return true;
}

ArchiveReader::ArchiveReader( ObjectResolver* resolver )
	: m_Resolver( resolver )
{

}

ArchiveReader::ArchiveReader( const FilePath& filePath, ObjectResolver* resolver )
	: Archive ( filePath )
	, m_Resolver( resolver )
{
}

ArchiveMode ArchiveReader::GetMode() const
{
	return ArchiveModes::Read;
}

bool ArchiveReader::Resolve( const Name& identity, ObjectPtr& pointer, const Class* pointerClass )
{
	if ( !m_Resolver || !m_Resolver->Resolve( identity, pointer, pointerClass ) )
	{
		uint32_t index = Invalid< uint32_t >();
		String str ( identity.Get() );

		Object* found = NULL;
		int parseSuccessful = str.Parse( "%d", &index );

		if ( !parseSuccessful )
		{
			HELIUM_TRACE(
				TraceLevels::Warning,
				"ArchiveReader::Resolve - Could not parse identity '%s' as a number.\n", 
				*str);
		}
		else if (index <= m_Objects.GetSize() )
		{
			found = m_Objects.GetElement( index );
		}

		if ( found )
		{
			if ( !found->IsClass( pointerClass ) )
			{
				Log::Warning( TXT( "Object of type '%s' is not valid for pointer type '%s'" ), pointer->GetClass()->m_Name, pointerClass->m_Name );
			}
			else
			{
				pointer = found;
			}
		}
		else // not found yet, must be later in the file, add a fixup to try again once the objects are done loading
		{
			m_Fixups.Push( Fixup ( identity, pointer, pointerClass ) );
		}
	}

	return true;
}

ArchiveWriterPtr Persist::GetWriter( const FilePath& path, ObjectIdentifier* identifier, ArchiveType archiveType )
{
	switch ( archiveType )
	{
	case ArchiveTypes::Auto:
		{
			if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::Json ] ) )
			{
				return new ArchiveWriterJson( path, identifier );
			}
			else if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::MessagePack ] ) )
			{
				return new ArchiveWriterMessagePack( path, identifier );
			}
			break;
		}

	case ArchiveTypes::Json:
		return new ArchiveWriterJson( path, identifier );

	case ArchiveTypes::MessagePack:
		return new ArchiveWriterMessagePack( path, identifier );

	default:
		HELIUM_ASSERT( false );
		break;
	}

	throw Persist::StreamException( TXT( "Unknown archive type" ) );
}

ArchiveReaderPtr Persist::GetReader( const FilePath& path, ObjectResolver* resolver, ArchiveType archiveType )
{
	switch ( archiveType )
	{
	case ArchiveTypes::Auto:
		{
			if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::Json ] ) )
			{
				return new ArchiveReaderJson( path, resolver );
			}
			else if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::MessagePack ] ) )
			{
				return new ArchiveReaderMessagePack( path, resolver );
			}
			break;
		}

	case ArchiveTypes::Json:
		return new ArchiveReaderJson( path, resolver );

	case ArchiveTypes::MessagePack:
		return new ArchiveReaderMessagePack( path, resolver );

	default:
		HELIUM_ASSERT( false );
		break;
	}

	throw Persist::StreamException( TXT( "Unknown archive type" ) );
}

bool Persist::ToArchive( const FilePath& path, ObjectPtr object, ObjectIdentifier* identifier, ArchiveType archiveType, std::string* error )
{
	HELIUM_ASSERT( !path.empty() );
	PERSIST_SCOPE_TIMER( ( "%s", path.c_str() ) );
	Log::Debug( TXT( "Generating '%s'\n" ), path.c_str() );

	path.MakePath();

	// build a path to a unique file for this process
	FilePath safetyPath( path.Directory() + Helium::GetProcessString() );
	safetyPath.ReplaceExtension( path.Extension() );

	ArchiveWriterPtr archive = GetWriter( safetyPath, identifier, archiveType );

	// generate the file to the safety location
	if ( Helium::IsDebuggerPresent() )
	{
		archive->Open();
		archive->Write( object );
		archive->Close();
	}
	else
	{
		bool open = false;

		try
		{
			archive->Open();
			open = true;
			archive->Write( object );
			archive->Close(); 
		}
		catch ( Helium::Exception& ex )
		{
			std::stringstream str;
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
		catch( ... )
		{
			if ( open )
			{
				archive->Close();
			}

			safetyPath.Delete();
			throw;
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
		std::stringstream str;
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

bool Persist::FromArchive( const FilePath& path, ObjectPtr& object, ObjectResolver* resolver, ArchiveType archiveType, std::string* error )
{
	HELIUM_ASSERT( !path.empty() );
	PERSIST_SCOPE_TIMER( ( "%s", path.c_str() ) );
	Log::Debug( TXT( "Parsing '%s'\n" ), path.c_str() );

	ArchiveReaderPtr archive = GetReader( path, resolver, archiveType );

	if ( Helium::IsDebuggerPresent() )
	{
		archive->Open();
		archive->Read( object );
		archive->Close(); 
	}
	else
	{
		bool open = false;

		try
		{
			archive->Open();
			open = true;
			archive->Read( object );
			archive->Close(); 
		}
		catch ( Helium::Exception& ex )
		{
			std::stringstream str;
			str << "While reading '" << path.c_str() << "': " << ex.Get();

			if ( error )
			{
				*error = str.str();
			}

			if ( open )
			{
				archive->Close();
			}

			return false;
		}
		catch ( ... )
		{
			if ( open )
			{
				archive->Close();
			}

			throw;
		}
	}

	return true;
}

ObjectPtr Persist::FromArchive( const FilePath& path, ObjectResolver* resolver, ArchiveType archiveType, std::string* error )
{
	ObjectPtr object;
	FromArchive( path, object, resolver, archiveType, error );
	return object;
}