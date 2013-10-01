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

#include "Persist/ArchiveBson.h"
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

Archive::Archive( uint32_t flags )
	: m_Progress( 0 )
	, m_Abort( false )
	, m_Flags( flags )
{
}

Archive::Archive( const FilePath& path, uint32_t flags )
	: m_Path( path )
	, m_Progress( 0 )
	, m_Abort( false )
	, m_Flags( flags )
{
	HELIUM_ASSERT( !m_Path.empty() );
}

Archive::~Archive()
{
}

SmartPtr< ArchiveWriter > ArchiveWriter::GetWriter( const FilePath& path, ObjectIdentifier* identifier, ArchiveType archiveType )
{
	switch ( archiveType )
	{
	case ArchiveTypes::Auto:
		{
			if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::Bson ] ) == 0 )
			{
				return new ArchiveWriterBson( path, identifier );
			}
			else if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::Json ] ) == 0 )
			{
				return new ArchiveWriterJson( path, identifier );
			}
			else if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::MessagePack ] ) == 0 )
			{
				return new ArchiveWriterMessagePack( path, identifier );
			}
			break;
		}

	case ArchiveTypes::Bson:
		return new ArchiveWriterBson( path, identifier );

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

bool ArchiveWriter::WriteToFile( const FilePath& path, const ObjectPtr& object, ObjectIdentifier* identifier, ArchiveType archiveType, std::string* error )
{
	return WriteToFile( path, &object, 1, identifier, archiveType, error );
}

bool ArchiveWriter::WriteToFile( const FilePath& path, const ObjectPtr* objects, size_t count, ObjectIdentifier* identifier, ArchiveType archiveType, std::string* error )
{
	HELIUM_ASSERT( !path.empty() );
	PERSIST_SCOPE_TIMER( ( "%s", path.c_str() ) );
	Log::Debug( TXT( "Generating '%s'\n" ), path.c_str() );

	path.MakePath();

	// build a path to a unique file for this process
	FilePath safetyPath( path.Directory() + Helium::GetProcessString() );
	safetyPath.ReplaceExtension( path.Extension() );

	SmartPtr< ArchiveWriter > archive = GetWriter( safetyPath, identifier, archiveType );

	// generate the file to the safety location
	if ( Helium::IsDebuggerPresent() )
	{
		archive->Open();
		archive->Write( objects, count );
		archive->Close();
	}
	else
	{
		bool open = false;

		try
		{
			archive->Open();
			open = true;
			archive->Write( objects, count );
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

ArchiveWriter::ArchiveWriter( ObjectIdentifier* identifier, uint32_t flags )
	: Archive( flags )
	, m_Identifier( identifier )
{

}

ArchiveWriter::ArchiveWriter( const FilePath& filePath, ObjectIdentifier* identifier, uint32_t flags )
	: Archive( filePath, flags )
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

SmartPtr< ArchiveReader > ArchiveReader::GetReader( const FilePath& path, ObjectResolver* resolver, ArchiveType archiveType )
{
	switch ( archiveType )
	{
	case ArchiveTypes::Auto:
		{
			if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::Bson ] ) == 0 )
			{
				return new ArchiveReaderBson( path, resolver );
			}
			else if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::Json ] ) == 0 )
			{
				return new ArchiveReaderJson( path, resolver );
			}
			else if ( CaseInsensitiveCompareString( path.Extension().c_str(), ArchiveExtensions[ ArchiveTypes::MessagePack ] ) == 0 )
			{
				return new ArchiveReaderMessagePack( path, resolver );
			}
			break;
		}

	case ArchiveTypes::Bson:
		return new ArchiveReaderBson( path, resolver );

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

bool ArchiveReader::ReadFromFile( const FilePath& path, ObjectPtr& object, ObjectResolver* resolver, ArchiveType archiveType, std::string* error )
{
	DynamicArray< ObjectPtr > objects;
	if ( ReadFromFile( path, objects, resolver, archiveType, error ) )
	{
		HELIUM_ASSERT( !objects.IsEmpty() );
		object = objects.GetFirst();
		return true;
	}

	return false;
}

bool ArchiveReader::ReadFromFile( const FilePath& path, DynamicArray< ObjectPtr >& objects, ObjectResolver* resolver, ArchiveType archiveType, std::string* error )
{
	HELIUM_ASSERT( !path.empty() );
	PERSIST_SCOPE_TIMER( ( "%s", path.c_str() ) );
	Log::Debug( TXT( "Parsing '%s'\n" ), path.c_str() );

	SmartPtr< ArchiveReader > archive = GetReader( path, resolver, archiveType );

	if ( Helium::IsDebuggerPresent() )
	{
		archive->Open();
		archive->Read( objects );
		archive->Close(); 
	}
	else
	{
		bool open = false;

		try
		{
			archive->Open();
			open = true;
			archive->Read( objects );
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

ObjectPtr ArchiveReader::ReadFromFile( const FilePath& path, ObjectResolver* resolver, ArchiveType archiveType, std::string* error )
{
	ObjectPtr object;
	ReadFromFile( path, object, resolver, archiveType, error );
	return object;
}

ArchiveReader::ArchiveReader( ObjectResolver* resolver, uint32_t flags )
	: Archive( flags )
	, m_Resolver( resolver )
{

}

ArchiveReader::ArchiveReader( const FilePath& filePath, ObjectResolver* resolver, uint32_t flags )
	: Archive ( filePath, flags )
	, m_Resolver( resolver )
{
}

ArchiveMode ArchiveReader::GetMode() const
{
	return ArchiveModes::Read;
}

Reflect::ObjectPtr ArchiveReader::AllocateObject( const Reflect::MetaClass* type )
{
	Object* object = type->m_Creator();

	// if we pre-allocated a proxy, hook it up to the object
	if ( m_Proxies.size() > m_Objects.GetSize() )
	{
		// find the appropriate pre-allocated proxy
		RefCountProxy< Object >* proxy = m_Proxies[ m_Objects.GetSize() ];

		// associate the object with the proxy
		proxy->SetObject( object );

		// associate the proxy with the object
		object->SetRefCountProxy( proxy );
	}

	return ObjectPtr( object );
}

bool ArchiveReader::Resolve( const Name& identity, ObjectPtr& pointer, const MetaClass* pointerClass )
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
				"ArchiveReader::Resolve - Could not parse identity '%s' as a number!\n", 
				*str);
		}
		else if ( index < m_Objects.GetSize() )
		{
			found = m_Objects.GetElement( index );
		}

		if ( found )
		{
			if ( !found->IsA( pointerClass ) )
			{
				Log::Warning( TXT( "Object of type '%s' is not valid for pointer type '%s'" ), pointer->GetMetaClass()->m_Name, pointerClass->m_Name );
			}
			else
			{
				pointer = found;
			}
		}
		else // not found yet, must be later in the file, add a fixup to try again once the objects are done loading
		{
			// ensure our list of proxies is sufficient size for this index
			if ( m_Proxies.size() < index+1 )
			{
				m_Proxies.resize( index+1 );
			}

			// ensure that we have allocated a proxy for this object
			RefCountProxy< Reflect::Object >* proxy = m_Proxies[ index ];
			if ( !proxy )
			{
				proxy = Object::RefCountSupportType::Allocate();
				MemorySet( proxy, 0 , sizeof( *proxy ) );
				m_Proxies[ index ] = proxy;
			}

			// release whatever we might already be pointing at and set the pointer to look at our pre-allocated proxy
			pointer.Release();
			pointer.SetProxy( reinterpret_cast< RefCountProxyBase< Reflect::Object >* >( proxy ) );

			// Make sure the proxy accounts for our reference
			proxy->AddStrongRef();

			// kick down the road the association of the proxy with the object (we will find it again by index)
			m_Fixups.Push( Fixup ( index, pointerClass ) );
		}
	}

	return true;
}

void ArchiveReader::Resolve()
{
	ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
	info.m_Progress = 100;
	e_Status.Raise( info );

#if HELIUM_TOOLS
	// finish linking objects (unless we have a custom handler)
	for ( DynamicArray< Fixup >::ConstIterator itr = m_Fixups.Begin(), end = m_Fixups.End(); itr != end; ++itr )
	{
		RefCountProxyBase< void >* proxy = m_Proxies[ itr->m_Index ];
		if ( proxy )
		{
			Object* found = m_Objects.GetElement( itr->m_Index );
			if ( HELIUM_VERIFY( found ) )
			{
				if ( !found->IsA( itr->m_PointerClass ) )
				{
					Log::Warning( TXT( "Object of type '%s' is not valid for pointer type '%s'" ), found->GetMetaClass()->m_Name, itr->m_PointerClass->m_Name );
				}
			}
		}
	}
#endif

	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}
