#include "PersistPch.h"
#include "Persist/ArchiveBinary.h"

#include "Foundation/Endian.h"
#include "Foundation/FileStream.h"

#include "Reflect/Object.h"
#include "Reflect/Composite.h"
#include "Reflect/Registry.h"
#include "Reflect/DataDeduction.h"

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;

ArchiveWriterBinary::ArchiveWriterBinary( const FilePath& path, Reflect::ObjectIdentifier* identifier )
	: ArchiveWriter( path, identifier )
{
}

ArchiveWriterBinary::ArchiveWriterBinary( Stream *stream, Reflect::ObjectIdentifier* identifier )
	: ArchiveWriter( identifier )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
	m_Writer.SetStream( stream );
}

ArchiveType ArchiveWriterBinary::GetType() const
{
	return ArchiveTypes::Binary;
}

void ArchiveWriterBinary::Open()
{
#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Opening file '%s'\n"), m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path, FileStream::MODE_WRITE );
	m_Stream.Reset( stream );
	m_Writer.SetStream( stream );
}

void ArchiveWriterBinary::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close(); 
	m_Stream.Release(); 
}

void ArchiveWriterBinary::Write()
{
	PERSIST_SCOPE_TIMER( ("Reflect - Binary Write") );

	ArchiveStatus info( *this, ArchiveStates::Starting );
	e_Status.Raise( info );

	// the master object
	m_Objects.Push( m_Object );

	// serialize main file objects
	{
		PERSIST_SCOPE_TIMER( ("Write Objects") );

		// objects can get changed during this iteration (in Identify), so use indices
		for ( int index = 0; index < m_Objects.GetSize(); ++index )
		{
			Object* object = m_Objects.GetElement( index );
			SerializeInstance( object, object->GetClass(), object );

			ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
			info.m_Progress = (int)(((float)(index) / (float)m_Objects.GetSize()) * 100.0f);
			e_Status.Raise( info );
		}

		ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
		info.m_Progress = 100;
		e_Status.Raise( info );
	}

	// do cleanup
	m_Stream->Flush();

	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveWriterBinary::SerializeInstance( void* instance, const Composite* composite, Object* object )
{
#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print( TXT( "Serializing %s\n" ), composite->m_Name );
#endif

	if ( m_Flags & ArchiveFlags::StringCrc )
	{
		// write the crc of the class of structure (used to factory allocate an instance when reading)
		uint32_t typeCrc = Crc32( composite->m_Name );
		m_Writer.Write( typeCrc );
	}
	else
	{
		// write the actual string
		m_Writer.Write( composite->m_Name );
	}

	if ( instance )
	{
#pragma TODO("Declare a max depth for inheritance to save heap allocs -geoff")
		DynamicArray< const Composite* > bases;
		for ( const Composite* current = composite; current != NULL; current = current->m_Base )
		{
			bases.Push( current );
		}

#pragma TODO("Declare a max count for fields to save heap allocs -geoff")
		DynamicArray< const Field* > fields;
		while ( !bases.IsEmpty() )
		{
			const Composite* current = bases.Pop();
			DynamicArray< Field >::ConstIterator itr = current->m_Fields.Begin();
			DynamicArray< Field >::ConstIterator end = current->m_Fields.End();
			for ( ; itr != end; ++itr )
			{
				const Field* field = &*itr;
				if ( field->ShouldSerialize( instance, object ) )
				{
					fields.Push( field );
				}
			}
		}

		m_Writer.BeginMap( static_cast< uint32_t >( fields.GetSize() ) );
		object->PreSerialize( NULL );

		DynamicArray< const Field* >::ConstIterator itr = fields.Begin();
		DynamicArray< const Field* >::ConstIterator end = fields.End();
		for ( ; itr != end; ++itr )
		{
			const Field* field = *itr;
			object->PreSerialize( field );
			SerializeField( instance, field, object );
			object->PostSerialize( field );
		}

		object->PostSerialize( NULL );
		m_Writer.EndMap();
	}
	else
	{
		m_Writer.WriteNil();
	}
}

void ArchiveWriterBinary::SerializeField( void* instance, const Reflect::Field* field, Object* object )
{
#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Serializing field %s\n"), field->m_Name);
#endif

	if ( m_Flags & ArchiveFlags::StringCrc )
	{
		// write the crc of the field name (used to associate a field when reading)
		uint32_t fieldNameCrc = Crc32( field->m_Name );
		m_Writer.Write( fieldNameCrc );
	}
	else
	{
		// write the actual string
		m_Writer.Write( field->m_Name );
	}

	switch ( field->m_Data->GetReflectionType() )
	{
	case ReflectionTypes::ScalarData:
		{
			break;
		}

	case ReflectionTypes::SetData:
		{
			break;
		}

	case ReflectionTypes::SequenceData:
		{
			break;
		}

	case ReflectionTypes::AssociationData:
		{
			break;
		}
	}
}

void ArchiveWriterBinary::ToStream( Object* object, Stream& stream, ObjectIdentifier* identifier, uint32_t flags )
{
	ArchiveWriterBinary archive ( &stream, identifier );
	archive.m_Object = object;
	archive.m_Flags = flags;
	archive.Write();   
	archive.Close(); 
}

ArchiveReaderBinary::ArchiveReaderBinary( const FilePath& path, ObjectResolver* resolver )
	: ArchiveReader( path, resolver )
	, m_Stream( NULL )
	, m_Size( 0 )
{
}

ArchiveReaderBinary::ArchiveReaderBinary( Stream *stream, ObjectResolver* resolver )
	: ArchiveReader( resolver )
	, m_Stream( NULL )
	, m_Size( 0 )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
	m_Reader.SetStream( stream );
}

ArchiveType ArchiveReaderBinary::GetType() const
{
	return ArchiveTypes::Binary;
}

void ArchiveReaderBinary::Open()
{
#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Opening file '%s'\n"), m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path, FileStream::MODE_READ );
	m_Stream.Reset( stream );
	m_Reader.SetStream( stream );
}

void ArchiveReaderBinary::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close(); 
	m_Stream.Release(); 
}

void ArchiveReaderBinary::Read()
{
	PERSIST_SCOPE_TIMER( ("Reflect - Binary Read") );

	ArchiveStatus info( *this, ArchiveStates::Starting );
	e_Status.Raise( info );

	m_Abort = false;

	// determine the size of the input stream
	m_Stream->Seek(0, SeekOrigins::End);
	m_Size = m_Stream->Tell();
	m_Stream->Seek(0, SeekOrigins::Begin);

	// fail on an empty input stream
	if ( m_Size == 0 )
	{
		throw Persist::StreamException( TXT( "Input stream is empty (%s)" ), m_Path.c_str() );
	}

	// deserialize main file objects
	{
		PERSIST_SCOPE_TIMER( ("Read Objects") );
		DeserializeArray( m_Objects );
	}

	// finish linking objects (unless we have a custom handler
	if ( !m_Resolver )
	{
		for ( DynamicArray< Fixup >::ConstIterator itr = m_Fixups.Begin(), end = m_Fixups.End(); itr != end; ++itr )
		{
			Resolve( itr->m_Identity, itr->m_Pointer, itr->m_PointerClass );
		}
	}

	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveReaderBinary::DeserializeArray( DynamicArray< ObjectPtr >& objects )
{
	const uint32_t startOffset = (uint32_t)m_Stream->Tell();

	int32_t element_count = -1;

#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Debug(TXT("Deserializing %d objects\n"), element_count);
#endif

	if (element_count > 0)
	{
		for (int i=0; i<element_count && !m_Abort; i++)
		{
			ObjectPtr object;

			// read type string
			uint32_t typeCrc = Helium::BeginCrc32();

			// A null type name CRC indicates that a null reference was serialized, so no type lookup needs to be performed.
			const Class* type = NULL;
			if ( typeCrc != 0 )
			{
				type = Reflect::Registry::GetInstance()->GetClass( typeCrc );
			}

			// read length info
			uint32_t length = 0;

			if ( typeCrc == 0 )
			{
				// skip it, but account for already reading the length from the stream
			}
			else
			{
				if (type)
				{
					// allocate instance by name
					object = Registry::GetInstance()->CreateInstance( type );
				}

				// if we failed
				if (!object.ReferencesObject())
				{
					// skip it, but account for already reading the length from the stream

					// if you see this, then data is being lost because:
					//  1 - a type was completely removed from the codebase
					//  2 - a type was not found because its type library is not registered
					Log::Warning( TXT( "Unable to create object of type %s, size %d, skipping...\n" ), type ? type->m_Name : TXT("Unknown"), length);
				}
			}

			if ( object.ReferencesObject() )
			{
				DeserializeInstance( object, object->GetClass(), object );

				if (object.ReferencesObject())
				{
					uint32_t current = (uint32_t)m_Stream->Tell();

					ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
					info.m_Progress = (int)(((float)(current - startOffset) / (float)m_Size) * 100.0f);
					e_Status.Raise( info );

					m_Abort |= info.m_Abort;
				}
			}

			objects.Push( object );
		}
	}

	ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
	info.m_Progress = 100;
	e_Status.Raise( info );
}

void ArchiveReaderBinary::DeserializeInstance( void* instance, const Composite* composite, Reflect::Object* object )
{
#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Deserializing %s\n"), composite->m_Name);
#endif

	object->PreDeserialize( NULL );

	DeserializeFields( instance, composite, object );

	object->PostDeserialize( NULL );
}

void ArchiveReaderBinary::DeserializeFields( void* instance, const Composite* composite, Reflect::Object* object )
{
	// read field count
	int32_t fieldCount = -1;

	for (int i=0; i<fieldCount; i++)
	{
		// read field name crc
		uint32_t fieldNameCrc = BeginCrc32();

		const Field* field = composite->FindFieldByName(fieldNameCrc);
		if ( field )
		{
			object->PreDeserialize( field );

			DeserializeField( instance, field, object );

			object->PostDeserialize( field );
		}
	}
}

void ArchiveReaderBinary::DeserializeField( void* instance, const Field* field, Object* object )
{
#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Deserializing field %s\n"), field->m_Name);
#endif

	switch ( field->m_Data->GetReflectionType() )
	{
	case ReflectionTypes::ScalarData:
		{
			break;
		}

	case ReflectionTypes::SetData:
		{
			break;
		}

	case ReflectionTypes::SequenceData:
		{
			break;
		}

	case ReflectionTypes::AssociationData:
		{
			break;
		}
	}
}

ObjectPtr ArchiveReaderBinary::FromStream( Stream& stream, ObjectResolver* resolver, uint32_t flags )
{
	ArchiveReaderBinary archive( &stream, resolver );
	archive.m_Flags = flags;
	archive.Read();
	archive.Close(); 
	return archive.m_Object;
}
