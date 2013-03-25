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

const uint32_t Persist::BINARY_CURRENT_VERSION = 8;

ArchiveWriterBinary::ArchiveWriterBinary( const FilePath& path, Reflect::ObjectIdentifier* identifier )
	: ArchiveWriter( path, identifier )
{
}

ArchiveWriterBinary::ArchiveWriterBinary( Stream *stream, Reflect::ObjectIdentifier* identifier )
	: ArchiveWriter( identifier )
{
	OpenStream( stream, false );
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
	OpenStream( stream, true );
}

void ArchiveWriterBinary::OpenStream( Stream* stream, bool cleanup )
{
	m_Stream.Reset( stream, !cleanup );
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

	// write version
	m_Stream->Write( BINARY_CURRENT_VERSION ); 

	// the master object
	m_Objects.Push( m_Object );

	// serialize main file objects
	{
		PERSIST_SCOPE_TIMER( ("Write Objects") );
		SerializeArray(m_Objects, ArchiveFlags::Status);
	}

	// do cleanup
	m_Stream->Flush();

	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveWriterBinary::SerializeArray( const DynamicArray< ObjectPtr >& objects, uint32_t flags )
{
	int32_t size = (int32_t)( objects.GetSize() );
	m_Stream->Write(&size); 

#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Serializing %d objects\n"), size);
#endif

	// objects can get changed during this iteration (in Identify), so use indices
	for ( int index = 0; index < objects.GetSize(); ++index )
	{
		Object* object = objects.GetElement( index );

		SerializeInstance( object, object->GetClass(), object );

		if ( flags & ArchiveFlags::Status )
		{
			ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
			info.m_Progress = (int)(((float)(index) / (float)size) * 100.0f);
			e_Status.Raise( info );
		}
	}

	if ( flags & ArchiveFlags::Status )
	{
		ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
		info.m_Progress = 100;
		e_Status.Raise( info );
	}

	const int32_t terminator = -1;
	m_Stream->Write(&terminator); 
}

void ArchiveWriterBinary::SerializeInstance( void* instance, const Composite* composite, Object* object )
{
	// write the crc of the class of structure (used to factory allocate an instance when reading)
	uint32_t typeCrc = Crc32( composite->m_Name );
	m_Stream->Write(&typeCrc); 

	// stub out the length we are about to write
	uint32_t startOffset = (uint32_t)m_Stream->Tell();
	m_Stream->Write(&startOffset); 

#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print( TXT( "Serializing %s\n" ), composite->m_Name );
#endif

	if ( instance )
	{
		object->PreSerialize( NULL );

		// push a new struct on the stack
		WriteFields data;
		data.m_Count = 0;
		data.m_CountOffset = m_Stream->Tell();
		m_FieldStack.Push(data);
		{
			// stub out the number of fields we are about to write
			m_Stream->Write(&m_FieldStack.GetLast().m_Count);

			// serialize each field of the composite instance
			SerializeFields(instance, composite, object);

			// seek back and write our count
			m_Stream->Seek(m_FieldStack.GetLast().m_CountOffset, SeekOrigins::Begin);
			m_Stream->Write(&m_FieldStack.GetLast().m_Count); 
		}
		m_FieldStack.Pop();

		object->PreSerialize( NULL );
	}

	// compute amound written
	uint32_t endOffset = (uint32_t)m_Stream->Tell();
	uint32_t length = endOffset - startOffset;

	// seek back and write written amount at start offset
	m_Stream->Seek(startOffset, SeekOrigins::Begin);
	m_Stream->Write(&length); 

	// seek back to the end of the stream
	m_Stream->Seek(0, SeekOrigins::End);
}

void ArchiveWriterBinary::SerializeFields( void* instance, const Reflect::Composite* composite, Object* object )
{
	DynamicArray< const Composite* > bases;
	for ( const Composite* current = composite; current != NULL; current = current->m_Base )
	{
		bases.Push( current );
	}

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
				object->PreSerialize( field );

				SerializeField( instance, field, object );

				object->PostSerialize( field );
			}
		}
	}

	const int32_t terminator = -1;
	m_Stream->Write(&terminator);
}

void ArchiveWriterBinary::SerializeField( void* instance, const Reflect::Field* field, Object* object )
{
	// write the crc of the field name (used to associate a field when reading)
	uint32_t fieldNameCrc = Crc32( field->m_Name );
	m_Stream->Write( &fieldNameCrc );

	// stub out the length we are about to write
	uint32_t startOffset = (uint32_t)m_Stream->Tell();
	m_Stream->Write(&startOffset); 

#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Serializing field %s\n"), field->m_Name);
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

	// compute amound written
	uint32_t endOffset = (uint32_t)m_Stream->Tell();
	uint32_t length = endOffset - startOffset;

	// seek back and write written amount at start offset
	m_Stream->Seek(startOffset, SeekOrigins::Begin);
	m_Stream->Write(&length); 

	// seek back to the end of the stream
	m_Stream->Seek(0, SeekOrigins::End);

	// we wrote a field, so increment our count
	HELIUM_ASSERT(m_FieldStack.GetSize() > 0);
	m_FieldStack.GetLast().m_Count++;
}

void ArchiveWriterBinary::ToStream( Object* object, Stream& stream, ObjectIdentifier* identifier )
{
	ArchiveWriterBinary archive ( &stream, identifier );
	archive.m_Object = object;
	archive.Write();   
	archive.Close(); 
}

ArchiveReaderBinary::ArchiveReaderBinary( const FilePath& path, ObjectResolver* resolver )
	: ArchiveReader( path, resolver )
	, m_Stream( NULL )
	, m_Version( 0 )
	, m_Size( 0 )
{
}

ArchiveReaderBinary::ArchiveReaderBinary( Stream *stream, ObjectResolver* resolver )
	: ArchiveReader( resolver )
	, m_Stream( NULL )
	, m_Version( 0 )
	, m_Size( 0 )
{
	OpenStream( stream, false );
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
	OpenStream( stream, true );
}

void ArchiveReaderBinary::OpenStream( Stream* stream, bool cleanup )
{
	m_Stream.Reset( stream, !cleanup );
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

	// read version
	m_Stream->Read( m_Version );
	if (m_Version != BINARY_CURRENT_VERSION)
	{
		throw Persist::StreamException( TXT( "Input stream version for '%s' is not what is currently supported (input: %d, current: %d)\n" ), m_Path.c_str(), m_Version, BINARY_CURRENT_VERSION); 
	}

	// deserialize main file objects
	{
		PERSIST_SCOPE_TIMER( ("Read Objects") );
		DeserializeArray(m_Objects, ArchiveFlags::Status);
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

void ArchiveReaderBinary::DeserializeArray( DynamicArray< ObjectPtr >& objects, uint32_t flags )
{
	uint32_t startOffset = (uint32_t)m_Stream->Tell();

	int32_t element_count = -1;
	m_Stream->Read(element_count); 

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
			m_Stream->Read(typeCrc);

			// A null type name CRC indicates that a null reference was serialized, so no type lookup needs to be performed.
			const Class* type = NULL;
			if ( typeCrc != 0 )
			{
				type = Reflect::Registry::GetInstance()->GetClass( typeCrc );
			}

			// read length info
			uint32_t length = 0;
			m_Stream->Read(length);

			if ( typeCrc == 0 )
			{
				// skip it, but account for already reading the length from the stream
				m_Stream->Seek(length - sizeof(uint32_t), SeekOrigins::Current);
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
					m_Stream->Seek(length - sizeof(uint32_t), SeekOrigins::Current);

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
					if ( flags & ArchiveFlags::Status )
					{
						uint32_t current = (uint32_t)m_Stream->Tell();

						ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
						info.m_Progress = (int)(((float)(current - startOffset) / (float)m_Size) * 100.0f);
						e_Status.Raise( info );

						m_Abort |= info.m_Abort;
					}
				}
			}

			objects.Push( object );
		}
	}

	if (!m_Abort)
	{
		int32_t terminator = -1;
		m_Stream->Read(terminator);
		if (terminator != -1)
		{
			throw Persist::Exception( TXT( "Unterminated object array block (%s)" ), m_Path.c_str() );
		}
	}

	if ( flags & ArchiveFlags::Status )
	{
		ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
		info.m_Progress = 100;
		e_Status.Raise( info );
	}
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
	int32_t fieldCount = -1;
	m_Stream->Read(fieldCount);    

	for (int i=0; i<fieldCount; i++)
	{
		// read field name crc
		uint32_t fieldNameCrc = BeginCrc32();
		m_Stream->Read( fieldNameCrc );

		// get current stream position
		uint32_t startOffset = (uint32_t)m_Stream->Tell();

		// read length info
		uint32_t length = 0;
		m_Stream->Read(length);

		const Field* field = composite->FindFieldByName(fieldNameCrc);
		if ( field )
		{
			object->PreDeserialize( field );

			DeserializeField( instance, field, object );

			object->PostDeserialize( field );
		}

		// regardless of what happened, set the stream state to the next field
		m_Stream->Seek(startOffset + length - sizeof(uint32_t), SeekOrigins::Begin);
	}

	int32_t terminator = -1;
	m_Stream->Read(terminator); 
	if (terminator != -1)
	{
		throw Persist::Exception( TXT( "Unterminated field array block (%s)" ), m_Path.c_str() );
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

ObjectPtr ArchiveReaderBinary::FromStream( Stream& stream, ObjectResolver* resolver )
{
	ArchiveReaderBinary archive( &stream, resolver );
	archive.Read();
	archive.Close(); 
	return archive.m_Object;
}
