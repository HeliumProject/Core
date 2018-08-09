#include "Precompile.h"
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

static int32_t g_InitCount = 0;

void Persist::Startup()
{
	if ( ++g_InitCount == 1 )
	{
		Reflect::Startup();
		ArchiveWriterJson::Startup();
		ArchiveReaderJson::Startup();
		ArchiveWriterMessagePack::Startup();
		ArchiveReaderMessagePack::Startup();
	}
}

void Persist::Shutdown()
{
	if ( --g_InitCount == 0 )
	{
		ArchiveWriterJson::Shutdown();
		ArchiveReaderJson::Shutdown();
		ArchiveWriterMessagePack::Shutdown();
		ArchiveReaderMessagePack::Shutdown();
		Reflect::Shutdown();
	}
}

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
	HELIUM_ASSERT( !m_Path.Empty() );
}

Archive::~Archive()
{
}

DynamicArray<ArchiveWriter::Registration> ArchiveWriter::s_Writers;

void ArchiveWriter::Register( const char* extension, Allocate allocator )
{
	for ( DynamicArray<Registration>::Iterator itr = s_Writers.Begin(), end = s_Writers.End(); itr != end; ++itr )
	{
		if ( CaseInsensitiveCompareString( itr->m_Extension, extension ) == 0 )
		{
			return;
		}
	}

	Registration type;
	CopyString( type.m_Extension, extension );
	type.m_Allocator = allocator;
	s_Writers.Add( type );
}

void ArchiveWriter::Unregister( const char* extension )
{
	if ( s_Writers.GetSize() != 0 )
	{
		for ( size_t i = s_Writers.GetSize() - 1; i != 0; --i )
		{
			if ( CaseInsensitiveCompareString( s_Writers[i].m_Extension, extension ) == 0 )
			{
				s_Writers.Remove( i );
			}
		}
	}
}

SmartPtr< ArchiveWriter > ArchiveWriter::GetWriter( const FilePath& path, ObjectIdentifier* identifier )
{
	for ( DynamicArray<Registration>::Iterator itr = s_Writers.Begin(), end = s_Writers.End(); itr != end; ++itr )
	{
		if ( CaseInsensitiveCompareString( itr->m_Extension, path.Extension().c_str() ) == 0 )
		{
			return itr->m_Allocator( path, identifier );
		}
	}

	throw Persist::StreamException( "Unknown archive type for extension %s", path.Extension().c_str() );
}

bool ArchiveWriter::WriteToFile( const FilePath& path, const ObjectPtr& object, ObjectIdentifier* identifier, std::string* error )
{
	return WriteToFile( path, &object, 1, identifier, error );
}

bool ArchiveWriter::WriteToFile( const FilePath& path, const ObjectPtr* objects, size_t count, ObjectIdentifier* identifier, std::string* error )
{
	HELIUM_ASSERT( !path.Empty() );
	HELIUM_PERSIST_SCOPE_TIMER( "%s", path.Data() );
	Log::Debug( "Generating '%s'\n", path.Data() );

	path.MakePath();

	// build a path to a unique file for this process
	FilePath safetyPath( path.Directory() + Helium::GetProcessString() );
	safetyPath.ReplaceExtension( path.Extension() );

	SmartPtr< ArchiveWriter > archive = GetWriter( safetyPath, identifier );

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
			str << "While writing '" << path.Data() << "': " << ex.Get();

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
		str << "While moving '" << safetyPath.Data() << "' to '" << path.Data() << "': " << ex.Get();

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

bool ArchiveWriter::Identify( const ObjectPtr& object, Name* identity )
{
	if ( m_Identifier )
	{
		return m_Identifier->Identify( object, identity );
	}

	bool strictOwnership = reinterpret_cast< RefCountProxy< Reflect::Object >* >( object.GetProxy() )->GetStrongRefCount() == 1;
	if ( !strictOwnership )
	{
		if ( identity )
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
			identity->Set( str );
		}

		return true;
	}

	return false;
}

DynamicArray<ArchiveReader::Registration> ArchiveReader::s_Readers;

void ArchiveReader::Register( const char* extension, Allocate allocator )
{
	for ( DynamicArray<Registration>::Iterator itr = s_Readers.Begin(), end = s_Readers.End(); itr != end; ++itr )
	{
		if ( CaseInsensitiveCompareString( itr->m_Extension, extension ) == 0 )
		{
			return;
		}
	}

	Registration type;
	CopyString( type.m_Extension, extension );
	type.m_Allocator = allocator;
	s_Readers.Add( type );
}

void ArchiveReader::Unregister( const char* extension )
{
	if ( s_Readers.GetSize() != 0 )
	{
		for ( size_t i = s_Readers.GetSize() - 1; i != 0; --i )
		{
			if ( CaseInsensitiveCompareString( s_Readers[i].m_Extension, extension ) == 0 )
			{
				s_Readers.Remove( i );
			}
		}
	}
}

SmartPtr< ArchiveReader > ArchiveReader::GetReader( const FilePath& path, ObjectResolver* resolver )
{
	for ( DynamicArray<Registration>::Iterator itr = s_Readers.Begin(), end = s_Readers.End(); itr != end; ++itr )
	{
		if ( CaseInsensitiveCompareString( itr->m_Extension, path.Extension().c_str() ) == 0 )
		{
			return itr->m_Allocator( path, resolver );
		}
	}

	throw Persist::StreamException( "Unknown archive type for extension %s", path.Extension().c_str() );
}

bool ArchiveReader::ReadFromFile( const FilePath& path, ObjectPtr& object, ObjectResolver* resolver, std::string* error )
{
	DynamicArray< ObjectPtr > objects;
	if ( ReadFromFile( path, objects, resolver, error ) )
	{
		HELIUM_ASSERT( !objects.IsEmpty() );
		object = objects.GetFirst();
		return true;
	}

	return false;
}

bool ArchiveReader::ReadFromFile( const FilePath& path, DynamicArray< ObjectPtr >& objects, ObjectResolver* resolver, std::string* error )
{
	HELIUM_ASSERT( !path.Empty() );
	HELIUM_PERSIST_SCOPE_TIMER( "%s", path.Data() );
	Log::Debug( "Parsing '%s'\n", path.Data() );

	SmartPtr< ArchiveReader > archive = GetReader( path, resolver );

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
			str << "While reading '" << path.Data() << "': " << ex.Get();

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

ObjectPtr ArchiveReader::ReadFromFile( const FilePath& path, ObjectResolver* resolver, std::string* error )
{
	ObjectPtr object;
	ReadFromFile( path, object, resolver, error );
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

Reflect::ObjectPtr ArchiveReader::AllocateObject( const Reflect::MetaClass* type, size_t index )
{
	Object* object = type->m_Creator();

	// if we pre-allocated a proxy, hook it up to the object
	if ( index < m_Proxies.size() )
	{
		// find the appropriate pre-allocated proxy
		RefCountProxy< Object >* proxy = m_Proxies[ index ];

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
			return false;
		}
		else if ( index < m_Objects.GetSize() )
		{
			found = m_Objects.GetElement( index );
		}

		if ( found )
		{
			if ( !found->IsA( pointerClass ) )
			{
				Log::Warning( "Object of type '%s' is not valid for pointer type '%s'", pointer->GetMetaClass()->m_Name, pointerClass->m_Name );
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

	// do any necessary object finalization here

	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}
