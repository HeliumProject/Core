#include "PersistPch.h"
#include "Persist/ArchiveBinary.h"

#include "Foundation/Endian.h"
#include "Foundation/FileStream.h"

#include "Reflect/Object.h"
#include "Reflect/Structure.h"
#include "Reflect/Registry.h"
#include "Reflect/DataDeduction.h"

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;

const uint32_t ArchiveBinary::CURRENT_VERSION = 8;

ArchiveBinary::ArchiveBinary( const FilePath& path )
: Archive( path )
, m_Stream( NULL )
, m_CleanupStream( false )
, m_Version( CURRENT_VERSION )
, m_Size( 0 )
{
}

ArchiveBinary::ArchiveBinary( Stream *stream, bool write /* = false */ )
: m_Stream( NULL )
, m_CleanupStream( false )
, m_Version( CURRENT_VERSION )
, m_Size( 0 )
{
	OpenStream(stream, write);
}

ArchiveBinary::~ArchiveBinary()
{
	if ( m_CleanupStream )
	{
		delete m_Stream;
	}
}

ArchiveType ArchiveBinary::GetType() const
{
	return ArchiveTypes::Binary;
}

void ArchiveBinary::Open( bool write )
{
#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Opening file '%s'\n"), m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open(  m_Path, write ? FileStream::MODE_WRITE : FileStream::MODE_READ );
	m_CleanupStream = true;

	OpenStream( stream, write );
}

void ArchiveBinary::OpenStream( Stream* stream, bool write )
{
	m_Stream = stream; 
	m_Mode = (write) ? ArchiveModes::Write : ArchiveModes::Read; 
}

void ArchiveBinary::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close(); 
	m_Stream = NULL; 
}

void ArchiveBinary::Read()
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
		throw Reflect::StreamException( TXT( "Input stream is empty (%s)" ), m_Path.c_str() );
	}

	// read version
	m_Stream->Read( m_Version );
	if (m_Version != CURRENT_VERSION)
	{
		throw Reflect::StreamException( TXT( "Input stream version for '%s' is not what is currently supported (input: %d, current: %d)\n" ), m_Path.c_str(), m_Version, CURRENT_VERSION); 
	}

	DynamicArray< ObjectPtr > objects;

	// deserialize main file objects
	{
		PERSIST_SCOPE_TIMER( ("Read Objects") );
		DeserializeArray(objects, ArchiveFlags::Status);
	}

#pragma TODO("Link objects")

	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveBinary::Write()
{
	PERSIST_SCOPE_TIMER( ("Reflect - Binary Write") );

	ArchiveStatus info( *this, ArchiveStates::Starting );
	e_Status.Raise( info );

	// write version
	HELIUM_ASSERT( m_Version == CURRENT_VERSION );
	m_Stream->Write( m_Version ); 

	DynamicArray< ObjectPtr > objects;

#pragma TODO("Find objects")

	// serialize main file objects
	{
		PERSIST_SCOPE_TIMER( ("Write Objects") );
		SerializeArray(objects, ArchiveFlags::Status);
	}

	// do cleanup
	m_Stream->Flush();

	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveBinary::SerializeInstance(Object* object)
{
	// write the crc of the class of object (used to factory allocate an instance when reading)
	uint32_t classCrc = 0;
	if ( object )
	{
		classCrc = Crc32( object->GetClass()->m_Name );
	}

	m_Stream->Write(&classCrc); 

	// stub out the length we are about to write
	uint32_t startOffset = static_cast< uint32_t >( m_Stream->Tell() );
	m_Stream->Write(&startOffset); 

#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print( TXT( "Serializing %s\n" ), object->GetClass()->m_Name );
#endif

	if ( object )
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

			// serialize each field of the object
			SerializeFields(object);

			// seek back and write our count
			m_Stream->Seek(m_FieldStack.GetLast().m_CountOffset, SeekOrigins::Begin);
			m_Stream->Write(&m_FieldStack.GetLast().m_Count); 
		}
		m_FieldStack.Pop();

		object->PostSerialize( NULL );
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

void ArchiveBinary::SerializeInstance( void* structure, const Structure* type )
{
	// write the crc of the class of structure (used to factory allocate an instance when reading)
	uint32_t typeCrc = Crc32( type->m_Name );
	m_Stream->Write(&typeCrc); 

	// stub out the length we are about to write
	uint32_t startOffset = (uint32_t)m_Stream->Tell();
	m_Stream->Write(&startOffset); 

#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print( TXT( "Serializing %s\n" ), type->m_Name );
#endif

	if ( structure )
	{
		// push a new struct on the stack
		WriteFields data;
		data.m_Count = 0;
		data.m_CountOffset = m_Stream->Tell();
		m_FieldStack.Push(data);
		{
			// stub out the number of fields we are about to write
			m_Stream->Write(&m_FieldStack.GetLast().m_Count);

			// serialize each field of the structure
			SerializeFields(structure, type);

			// seek back and write our count
			m_Stream->Seek(m_FieldStack.GetLast().m_CountOffset, SeekOrigins::Begin);
			m_Stream->Write(&m_FieldStack.GetLast().m_Count); 
		}
		m_FieldStack.Pop();
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

void ArchiveBinary::SerializeFields( Object* object )
{
	const Composite* composite = object->GetClass();

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

			if ( field->ShouldSerialize( object ) )
			{
				uint32_t fieldNameCrc = Crc32( field->m_Name );
				m_Stream->Write( &fieldNameCrc ); 

#ifdef PERSIST_ARCHIVE_VERBOSE
				Log::Print(TXT("Serializing field %s\n"), field->m_Name);
#endif

				object->PreSerialize( field );

				switch ( field->m_Data->GetReflectionType() )
				{
				case ReflectionTypes::ScalarData:
					{
						ScalarData* data = static_cast< ScalarData* >( field->m_Data );
						data->Serialize( DataInstance( object, field ), *m_Stream );
						break;
					}
#pragma TODO("Containers")
				case ReflectionTypes::SetData:
					{
						break;
					}

				case ReflectionTypes::SequenceData:
					{
						break;
					}

				case ReflectionTypes::AssociativeData:
					{
						break;
					}
				}
				
				object->PostSerialize( field );

				// we wrote a field, so increment our count
				HELIUM_ASSERT(m_FieldStack.GetSize() > 0);
				m_FieldStack.GetLast().m_Count++;
			}
		}
	}

	const int32_t terminator = -1;
	m_Stream->Write(&terminator); 
}

void ArchiveBinary::SerializeFields( void* instance, const Reflect::Structure* composite )
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

			if ( field->ShouldSerialize( instance ) )
			{
				uint32_t fieldNameCrc = Crc32( field->m_Name );
				m_Stream->Write( &fieldNameCrc ); 

#ifdef PERSIST_ARCHIVE_VERBOSE
				Log::Print(TXT("Serializing field %s\n"), field->m_Name);
#endif

				switch ( field->m_Data->GetReflectionType() )
				{
				case ReflectionTypes::ScalarData:
					{
						ScalarData* data = static_cast< ScalarData* >( field->m_Data );
						data->Serialize( DataInstance( instance, field ), *m_Stream );
						break;
					}
#pragma TODO("Containers")
				case ReflectionTypes::SetData:
					{
						break;
					}

				case ReflectionTypes::SequenceData:
					{
						break;
					}

				case ReflectionTypes::AssociativeData:
					{
						break;
					}
				}

				// we wrote a field, so increment our count
				HELIUM_ASSERT(m_FieldStack.GetSize() > 0);
				m_FieldStack.GetLast().m_Count++;
			}
		}
	}

	const int32_t terminator = -1;
	m_Stream->Write(&terminator);
}

void ArchiveBinary::SerializeArray( const DynamicArray< ObjectPtr >& objects, uint32_t flags )
{
	int32_t size = (int32_t)( objects.GetSize() );
	m_Stream->Write(&size); 

#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Serializing %d objects\n"), size);
#endif

	DynamicArray< ObjectPtr >::ConstIterator itr = objects.Begin(), end = objects.End();
	for (int index = 0; itr != end; ++itr, ++index )
	{
		SerializeInstance(*itr);

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

void ArchiveBinary::DeserializeInstance(ObjectPtr& object)
{
	//
	// If we don't have an object allocated for deserialization, pull one from the stream
	//

	if (!object.ReferencesObject())
	{
		object = Allocate();
	}

	//
	// We should now have an instance (unless data was skipped)
	//

	if (object.ReferencesObject())
	{
#ifdef PERSIST_ARCHIVE_VERBOSE
		Log::Print(TXT("Deserializing %s\n"), object->GetClass()->m_Name);
#endif

		object->PreDeserialize( NULL );

		DeserializeFields(object);

		object->PostDeserialize( NULL );
	}
}

void ArchiveBinary::DeserializeInstance( void* structure, const Structure* type )
{
#ifdef PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Deserializing %s\n"), type->m_Name);
#endif

	DeserializeFields(structure, type);
}

void ArchiveBinary::DeserializeFields(Object* object)
{
	int32_t fieldCount = -1;
	m_Stream->Read(fieldCount); 
	
	for (int i=0; i<fieldCount; i++)
	{
		uint32_t fieldNameCrc = BeginCrc32();
		m_Stream->Read( fieldNameCrc );

		const Class* type = object->GetClass();
		HELIUM_ASSERT( type );

		ObjectPtr unknown;

		const Field* field = type->FindFieldByName(fieldNameCrc);
		if ( field )
		{
#ifdef PERSIST_ARCHIVE_VERBOSE
			Log::Print(TXT("Deserializing field %s\n"), field->m_Name);
#endif
			switch ( field->m_Data->GetReflectionType() )
			{
			case ReflectionTypes::ScalarData:
				{
					ScalarData* data = static_cast< ScalarData* >( field->m_Data );
					data->Deserialize( DataInstance( object, field ), *m_Stream, /* raiseChanged = */ false );
					break;
				}
#pragma TODO("Containers")
			case ReflectionTypes::SetData:
				{
					break;
				}

			case ReflectionTypes::SequenceData:
				{
					break;
				}

			case ReflectionTypes::AssociativeData:
				{
					break;
				}
			}
		}
		else // else the field does not exist in the current class anymore
		{
			try
			{
				DeserializeInstance( unknown );
			}
			catch (Reflect::LogisticException& ex)
			{
				Log::Debug( TXT( "Unable to deserialize %s::%s, discarding: %s\n" ), type->m_Name, field->m_Name, ex.What());
			}
		}

		if ( unknown.ReferencesObject() )
		{
			// attempt processing
			object->ProcessUnknown( unknown, field ? Crc32( field->m_Name ) : 0 );
		}
	}

	int32_t terminator = -1;
	m_Stream->Read(terminator); 
	if (terminator != -1)
	{
		throw Reflect::DataFormatException( TXT( "Unterminated field array block (%s)" ), m_Path.c_str() );
	}
}

void ArchiveBinary::DeserializeFields( void* instance, const Structure* type )
{
	int32_t fieldCount = -1;
	m_Stream->Read(fieldCount);    

	for (int i=0; i<fieldCount; i++)
	{
		uint32_t fieldNameCrc = BeginCrc32();
		m_Stream->Read( fieldNameCrc );

		const Field* field = type->FindFieldByName(fieldNameCrc);
		if ( field )
		{
#ifdef PERSIST_ARCHIVE_VERBOSE
			Log::Print(TXT("Deserializing field %s\n"), field->m_Name);
#endif
			switch ( field->m_Data->GetReflectionType() )
			{
			case ReflectionTypes::ScalarData:
				{
					ScalarData* data = static_cast< ScalarData* >( field->m_Data );
					data->Deserialize( DataInstance( instance, field ), *m_Stream, /* raiseChanged = */ false );
					break;
				}
#pragma TODO("Containers")
			case ReflectionTypes::SetData:
				{
					break;
				}

			case ReflectionTypes::SequenceData:
				{
					break;
				}

			case ReflectionTypes::AssociativeData:
				{
					break;
				}
			}
		}
	}

	int32_t terminator = -1;
	m_Stream->Read(terminator); 
	if (terminator != -1)
	{
		throw Reflect::DataFormatException( TXT( "Unterminated field array block (%s)" ), m_Path.c_str() );
	}
}

void ArchiveBinary::DeserializeArray( DynamicArray< ObjectPtr >& objects, uint32_t flags )
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
			ObjectPtr object = Allocate();

			DeserializeInstance(object);

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

			objects.Push( object );
		}
	}

	if (!m_Abort)
	{
		int32_t terminator = -1;
		m_Stream->Read(terminator);
		if (terminator != -1)
		{
			throw Reflect::DataFormatException( TXT( "Unterminated object array block (%s)" ), m_Path.c_str() );
		}
	}

	if ( flags & ArchiveFlags::Status )
	{
		ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
		info.m_Progress = 100;
		e_Status.Raise( info );
	}
}

ObjectPtr ArchiveBinary::Allocate()
{
	ObjectPtr object;

	// read type string
	uint32_t typeCrc = Helium::BeginCrc32();
	m_Stream->Read(typeCrc);

	// A null type name CRC indicates that a null reference was serialized, so no type lookup needs to be performed.
	const Class* reflect_class = NULL;
	const Type* reflect_type = NULL;
	if ( typeCrc != 0 )
	{
		reflect_class = Reflect::Registry::GetInstance()->GetClass( typeCrc );
		reflect_type = Reflect::Registry::GetInstance()->GetType( typeCrc );
	}

	// read length info if we have it
	uint32_t length = 0;
	m_Stream->Read(length);

	if ( typeCrc == 0 )
	{
		// skip it, but account for already reading the length from the stream
		m_Stream->Seek(length - sizeof(uint32_t), SeekOrigins::Current);
	}
	else
	{
		if (reflect_class)
		{
			// allocate instance by name
			object = Registry::GetInstance()->CreateInstance( reflect_class );
		}

		// if we failed
		if (!object.ReferencesObject())
		{
			if (!reflect_class && reflect_type)
			{
				// It's a struct, don't worry about it
			}
			else
			{
				// skip it, but account for already reading the length from the stream
				m_Stream->Seek(length - sizeof(uint32_t), SeekOrigins::Current);

				// if you see this, then data is being lost because:
				//  1 - a type was completely removed from the codebase
				//  2 - a type was not found because its type library is not registered
				Log::Debug( TXT( "Unable to create object of type %s, size %d, skipping...\n" ), reflect_class ? reflect_class->m_Name : TXT("Unknown"), length);
	#pragma TODO("Support blind data")
			}
		}
	}

	return object;
}

void ArchiveBinary::ToStream( Object* object, Stream& stream )
{
	ArchiveBinary archive ( &stream, true );
	archive.m_Object = object;
	archive.Write();   
	archive.Close(); 
}

ObjectPtr ArchiveBinary::FromStream( Stream& stream )
{
	ArchiveBinary archive( &stream, false );
	archive.Read();
	archive.Close(); 
	return archive.m_Object;
}
