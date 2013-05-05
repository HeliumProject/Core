#include "PersistPch.h"
#include "Persist/ArchiveJson.h"

#include "Foundation/Endian.h"
#include "Foundation/FileStream.h"

#include "Reflect/Object.h"
#include "Reflect/Structure.h"
#include "Reflect/Registry.h"
#include "Reflect/TranslatorDeduction.h"

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;

ArchiveWriterJson::ArchiveWriterJson( const FilePath& path, ObjectIdentifier* identifier )
	: ArchiveWriter( path, identifier )
{
}

ArchiveWriterJson::ArchiveWriterJson( Stream *stream, ObjectIdentifier* identifier )
	: ArchiveWriter( identifier )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
}

ArchiveType ArchiveWriterJson::GetType() const
{
	return ArchiveTypes::Json;
}

void ArchiveWriterJson::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Opening file '%s'\n"), m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path, FileStream::MODE_WRITE );
	m_Stream.Reset( stream );
}

void ArchiveWriterJson::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close(); 
	m_Stream.Release(); 
}

void ArchiveWriterJson::Write()
{
	PERSIST_SCOPE_TIMER( ("Reflect - Json Write") );

	// notify starting
	ArchiveStatus info( *this, ArchiveStates::Starting );
	e_Status.Raise( info );

	// the master object
	m_Objects.Push( m_Object );

	// begin top level array of objects
	//m_Writer.BeginArray();

	// objects can get changed during this iteration (in Identify), so use indices
	for ( int index = 0; index < m_Objects.GetSize(); ++index )
	{
		Object* object = m_Objects.GetElement( index );
		const Class* objectClass = object->GetClass();

		//m_Writer.BeginMap( 1 );
		//m_Writer.Write( objectClass->m_Name );
		SerializeInstance( object, objectClass, object );
		//m_Writer.EndMap();

		info.m_State = ArchiveStates::ObjectProcessed;
		info.m_Progress = (int)(((float)(index) / (float)m_Objects.GetSize()) * 100.0f);
		e_Status.Raise( info );
	}

	// end top level array
	//m_Writer.EndArray();

	// notify completion of last object processed
	info.m_State = ArchiveStates::ObjectProcessed;
	info.m_Progress = 100;
	e_Status.Raise( info );

	// do cleanup
	m_Stream->Flush();

	// notify completion
	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveWriterJson::SerializeInstance( void* instance, const Structure* structure, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print( TXT( "Serializing %s\n" ), structure->m_Name );
#endif

#pragma TODO("Declare a max depth for inheritance to save heap allocs -geoff")
	DynamicArray< const Structure* > bases;
	for ( const Structure* current = structure; current != NULL; current = current->m_Base )
	{
		bases.Push( current );
	}

#pragma TODO("Declare a max count for fields to save heap allocs -geoff")
	DynamicArray< const Field* > fields;
	while ( !bases.IsEmpty() )
	{
		const Structure* current = bases.Pop();
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

	//m_Writer.BeginMap( static_cast< uint32_t >( fields.GetSize() ) );
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
	//m_Writer.EndMap();
}

void ArchiveWriterJson::SerializeField( void* instance, const Field* field, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Serializing field %s\n"), field->m_Name);
#endif

	// write the actual string
	//m_Writer.Write( field->m_Name );

	if ( field->m_Count > 1 )
	{
		//m_Writer.BeginArray( field->m_Count );

		for ( uint32_t i=0; i<field->m_Count; ++i )
		{
			SerializeTranslator( Pointer ( field, object, i ), field->m_Translator, field, object );
		}

		//m_Writer.EndArray();
	}
	else
	{
		SerializeTranslator( Pointer ( field, object ), field->m_Translator, field, object );
	}
}

void ArchiveWriterJson::SerializeTranslator( Pointer pointer, Translator* translator, const Field* field, Object* object )
{
	switch ( translator->GetReflectionType() )
	{
	case ReflectionTypes::ScalarTranslator:
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			switch ( scalar->m_Type )
			{
			case ScalarTypes::Boolean:
				//m_Writer.Write( pointer.As<bool>() );
				break;

			case ScalarTypes::Unsigned8:
				//m_Writer.Write( pointer.As<uint8_t>() );
				break;

			case ScalarTypes::Unsigned16:
				//m_Writer.Write( pointer.As<uint16_t>() );
				break;

			case ScalarTypes::Unsigned32:
				//m_Writer.Write( pointer.As<uint32_t>() );
				break;

			case ScalarTypes::Unsigned64:
				//m_Writer.Write( pointer.As<uint64_t>() );
				break;

			case ScalarTypes::Signed8:
				//m_Writer.Write( pointer.As<int8_t>() );
				break;

			case ScalarTypes::Signed16:
				//m_Writer.Write( pointer.As<int16_t>() );
				break;

			case ScalarTypes::Signed32:
				//m_Writer.Write( pointer.As<int32_t>() );
				break;

			case ScalarTypes::Signed64:
				//m_Writer.Write( pointer.As<int64_t>() );
				break;

			case ScalarTypes::Float32:
				//m_Writer.Write( pointer.As<float32_t>() );
				break;

			case ScalarTypes::Float64:
				//m_Writer.Write( pointer.As<float64_t>() );
				break;

			case ScalarTypes::String:
				String str;
				scalar->Print( pointer, str, *this );
				//m_Writer.Write( str.GetTranslator() );
				break;
			}
			break;
		}

	case ReflectionTypes::StructureTranslator:
		{
			StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
			SerializeInstance( pointer.m_Address, structure->GetStructure(), object );
			break;
		}

	case ReflectionTypes::SetTranslator:
		{
			SetTranslator* set = static_cast< SetTranslator* >( translator );

			Translator* itemTranslator = set->GetItemTranslator();
			DynamicArray< Pointer > items;
			set->GetItems( pointer, items );

			uint32_t length = static_cast< uint32_t >( items.GetSize() );
			//m_Writer.BeginArray( length );

			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr )
			{
				SerializeTranslator( *itr, itemTranslator, field, object );
			}

			//m_Writer.EndArray();

			break;
		}

	case ReflectionTypes::SequenceTranslator:
		{
			SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );

			Translator* itemTranslator = sequence->GetItemTranslator();
			DynamicArray< Pointer > items;
			sequence->GetItems( pointer, items );

			uint32_t length = static_cast< uint32_t >( items.GetSize() );
			//m_Writer.BeginArray( length );

			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr )
			{
				SerializeTranslator( *itr, itemTranslator, field, object );
			}

			//m_Writer.EndArray();

			break;
		}

	case ReflectionTypes::AssociationTranslator:
		{
			AssociationTranslator* association = static_cast< AssociationTranslator* >( translator );

			Translator* keyTranslator = association->GetKeyTranslator();
			Translator* valueTranslator = association->GetValueTranslator();
			DynamicArray< Pointer > keys, values;
			association->GetItems( pointer, keys, values );

			uint32_t length = static_cast< uint32_t >( keys.GetSize() );
			//m_Writer.BeginMap( length );

			for ( DynamicArray< Pointer >::Iterator keyItr = keys.Begin(), valueItr = values.Begin(), keyEnd = keys.End(), valueEnd = values.End();
				keyItr != keyEnd && valueItr != valueEnd;
				++keyItr, ++valueItr )
			{
				SerializeTranslator( *keyItr, keyTranslator, field, object );
				SerializeTranslator( *valueItr, valueTranslator, field, object );
			}

			//m_Writer.EndMap();

			break;
		}
	}
}

void ArchiveWriterJson::ToStream( Object* object, Stream& stream, ObjectIdentifier* identifier, uint32_t flags )
{
	ArchiveWriterJson archive ( &stream, identifier );
	archive.m_Object = object;
	archive.m_Flags = flags;
	archive.Write();   
	archive.Close(); 
}

ArchiveReaderJson::ArchiveReaderJson( const FilePath& path, ObjectResolver* resolver )
	: ArchiveReader( path, resolver )
	, m_Stream( NULL )
	, m_Size( 0 )
{
}

ArchiveReaderJson::ArchiveReaderJson( Stream *stream, ObjectResolver* resolver )
	: ArchiveReader( resolver )
	, m_Stream( NULL )
	, m_Size( 0 )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
}

ArchiveType ArchiveReaderJson::GetType() const
{
	return ArchiveTypes::Json;
}

void ArchiveReaderJson::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Opening file '%s'\n"), m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path, FileStream::MODE_READ );
	m_Stream.Reset( stream );
}

void ArchiveReaderJson::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close(); 
	m_Stream.Release(); 
}

void ArchiveReaderJson::Read()
{
	PERSIST_SCOPE_TIMER( ("Reflect - Json Read") );

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

	const int64_t startOffset = m_Stream->Tell();

	// parse the first byte of the stream
	//m_Reader.Advance();

#if 0
	if ( m_Reader.IsArray() )
	{
		uint32_t length = m_Reader.ReadArrayLength();
		m_Objects.Reserve( length );

		//m_Reader.BeginArray( length );

		for ( uint32_t i=0; i<length; i++ )
		{
			if ( m_Reader.IsMap() )
			{
				uint32_t length = m_Reader.ReadMapLength();
				if ( length == 1 )
				{
					m_Reader.BeginMap( length );

					uint32_t objectClassCrc = 0;
					if ( m_Reader.IsNumber() )
					{
						m_Reader.Read( objectClassCrc );
					}
					else
					{
						String typeStr;
						m_Reader.Read( typeStr );
						objectClassCrc = Helium::Crc32( typeStr.GetTranslator() );
					}

					m_Reader.Advance();

					const Class* objectClass = NULL;
					if ( objectClassCrc != 0 )
					{
						objectClass = Registry::GetInstance()->GetClass( objectClassCrc );
					}

					ObjectPtr object;
					if ( objectClass )
					{
						object = Registry::GetInstance()->CreateInstance( objectClass );
					}

					m_Objects.Push( object );

					if ( object.ReferencesObject() )
					{
						DeserializeInstance( object, object->GetClass(), object );

						int64_t current = m_Stream->Tell();

						info.m_State = ArchiveStates::ObjectProcessed;
						info.m_Progress = (int)(((float)(current - startOffset) / (float)m_Size) * 100.0f);
						e_Status.Raise( info );

						m_Abort |= info.m_Abort;
					}
					else // object.ReferencesObject()
					{
						m_Reader.Skip();
					}

					m_Reader.EndMap();
				}
				else // length == 1
				{
					m_Reader.Skip();
				}
			}
			else // IsMap
			{
				m_Reader.Skip();
			}
		} // for

		m_Reader.EndArray();
	}
#endif

	info.m_State = ArchiveStates::ObjectProcessed;
	info.m_Progress = 100;
	e_Status.Raise( info );

	// finish linking objects (unless we have a custom handler)
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

void ArchiveReaderJson::DeserializeInstance( void* instance, const Structure* structure, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Deserializing %s\n"), structure->m_Name);
#endif

	object->PreDeserialize( NULL );
#if 0
	if ( m_Reader.IsMap() )
	{
		uint32_t length = m_Reader.ReadMapLength();
		for (uint32_t i=0; i<length; i++)
		{
			uint32_t fieldNameCrc = BeginCrc32();

			uint32_t fieldCrc = 0;
			if ( m_Reader.IsNumber() )
			{
				m_Reader.Read( fieldCrc );
			}
			else
			{
				String fieldStr;
				m_Reader.Read( fieldStr );
				fieldCrc = Helium::Crc32( fieldStr.GetTranslator() );
			}

			m_Reader.Advance();

			const Field* field = structure->FindFieldByName( fieldNameCrc );
			if ( field )
			{
				object->PreDeserialize( field );

				DeserializeField( instance, field, object );

				object->PostDeserialize( field );
			}
			else
			{
				m_Reader.Skip();
			}
		} // for
	}
	else // IsMap()
	{
		m_Reader.Skip();
	}
#endif
	object->PostDeserialize( NULL );
}

void ArchiveReaderJson::DeserializeField( void* instance, const Field* field, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Deserializing field %s\n"), field->m_Name);
#endif
	
	if ( field->m_Count > 1 )
	{
#if 0
		if ( m_Reader.IsArray() )
		{
			uint32_t length = m_Reader.ReadArrayLength();
			m_Reader.BeginArray( length );
			for ( uint32_t i=0; i<length; ++i )
			{
				if ( i < field->m_Count )
				{
					DeserializeTranslator( Pointer ( field, object, i ), field->m_Translator, field, object );
				}
				else
				{
					m_Reader.Skip();
				}
			}
			m_Reader.EndArray();
		}
		else
		{
			DeserializeTranslator( Pointer ( field, object, 0 ), field->m_Translator, field, object );
		}
#endif
	}
	else
	{
		DeserializeTranslator( Pointer ( field, object ), field->m_Translator, field, object );
	}
}

void ArchiveReaderJson::DeserializeTranslator( Pointer pointer, Translator* translator, const Field* field, Object* object )
{
#if 0
	if ( m_Reader.IsBoolean() )
	{
		if ( translator->GetReflectionType() == ReflectionTypes::ScalarTranslator )
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			if ( scalar->m_Type == ScalarTypes::Boolean )
			{
				m_Reader.Read( pointer.As<bool>() );
			}
			else
			{
				m_Reader.Skip(); // no implicit conversion, discard data
			}
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else if ( m_Reader.IsNumber() )
	{
		if ( translator->GetReflectionType() == ReflectionTypes::ScalarTranslator )
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			bool clamp = true;
			switch ( scalar->m_Type )
			{
			case ScalarTypes::Unsigned8:
				m_Reader.ReadNumber( pointer.As<uint8_t>(), clamp );
				break;

			case ScalarTypes::Unsigned16:
				m_Reader.ReadNumber( pointer.As<uint16_t>(), clamp );
				break;

			case ScalarTypes::Unsigned32:
				m_Reader.ReadNumber( pointer.As<uint32_t>(), clamp );
				break;

			case ScalarTypes::Unsigned64:
				m_Reader.ReadNumber( pointer.As<uint64_t>(), clamp );
				break;

			case ScalarTypes::Signed8:
				m_Reader.ReadNumber( pointer.As<int8_t>(), clamp );
				break;

			case ScalarTypes::Signed16:
				m_Reader.ReadNumber( pointer.As<int16_t>(), clamp );
				break;

			case ScalarTypes::Signed32:
				m_Reader.ReadNumber( pointer.As<int32_t>(), clamp );
				break;

			case ScalarTypes::Signed64:
				m_Reader.ReadNumber( pointer.As<int64_t>(), clamp );
				break;

			case ScalarTypes::Float32:
				m_Reader.ReadNumber( pointer.As<float32_t>(), clamp );
				break;

			case ScalarTypes::Float64:
				m_Reader.ReadNumber( pointer.As<float64_t>(), clamp );
				break;

			default:
				m_Reader.Skip(); // no implicit conversion, discard data
				break;
			}
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else if ( m_Reader.IsRaw() )
	{
		if ( translator->GetReflectionType() == ReflectionTypes::ScalarTranslator )
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			if ( scalar->m_Type == ScalarTypes::String )
			{
				String str;
				m_Reader.Read( str );
				scalar->Parse( str, pointer, *this, m_Flags | ArchiveFlags::Notify ? true : false );
			}
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else if ( m_Reader.IsArray() )
	{
		if ( translator->GetReflectionType() == ReflectionTypes::SetTranslator )
		{
			SetTranslator* set = static_cast< SetTranslator* >( translator );
			Translator* itemTranslator = set->GetItemTranslator();
			uint32_t length = m_Reader.ReadArrayLength();
			for ( uint32_t i=0; i<length; ++i )
			{
				Variable item ( itemTranslator );
				DeserializeTranslator( item, itemTranslator, field, object );
				set->InsertItem( pointer, item );
			}
		}
		else if ( translator->GetReflectionType() == ReflectionTypes::SequenceTranslator )
		{
			SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );
			Translator* itemTranslator = sequence->GetItemTranslator();
			uint32_t length = m_Reader.ReadArrayLength();
			for ( uint32_t i=0; i<length; ++i )
			{
				Variable item ( itemTranslator );
				DeserializeTranslator( item, itemTranslator, field, object );
				sequence->SetItem( pointer, i, item );
			}
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else if ( m_Reader.IsMap() )
	{
		if ( translator->GetReflectionType() == ReflectionTypes::StructureTranslator )
		{
			StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
			DeserializeInstance( pointer.m_Address,  structure->GetStructure(), object );
		}
		else if ( translator->GetReflectionType() == ReflectionTypes::AssociationTranslator )
		{
			AssociationTranslator* assocation = static_cast< AssociationTranslator* >( translator );
			Translator* keyTranslator = assocation->GetKeyTranslator();
			Translator* valueTranslator = assocation->GetValueTranslator();
			Variable key ( keyTranslator );
			Variable value ( valueTranslator );
			uint32_t length = m_Reader.ReadMapLength();
			for ( uint32_t i=0; i<length; ++i )
			{
				DeserializeTranslator( key, keyTranslator, field, object );
				DeserializeTranslator( value, valueTranslator, field, object );
				assocation->SetItem( pointer, key, value );
			}
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else
	{
		m_Reader.Skip(); // no implicit conversion, discard data
	}
#endif
}

ObjectPtr ArchiveReaderJson::FromStream( Stream& stream, ObjectResolver* resolver, uint32_t flags )
{
	ArchiveReaderJson archive( &stream, resolver );
	archive.m_Flags = flags;
	archive.Read();
	archive.Close(); 
	return archive.m_Object;
}
