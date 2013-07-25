#include "PersistPch.h"
#include "Persist/ArchiveBson.h"

#include "Foundation/Endian.h"
#include "Foundation/FileStream.h"
#include "Foundation/Numeric.h"

#include "Reflect/Object.h"
#include "Reflect/MetaStruct.h"
#include "Reflect/Registry.h"
#include "Reflect/TranslatorDeduction.h"

REFLECT_DEFINE_BASE_STRUCT( Helium::Persist::BsonDate );
REFLECT_DEFINE_BASE_STRUCT( Helium::Persist::BsonObjectId );

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;

BsonObjectId BsonObjectId::Null;

void BsonDate::PopulateMetaType( Reflect::MetaStruct& type )
{
	type.AddField( &BsonDate::millis, "millis" );
}

void BsonObjectId::PopulateMetaType( Reflect::MetaStruct& type )
{
	type.AddField( &BsonObjectId::bytes, "bytes" );
}

ArchiveWriterBson::ArchiveWriterBson( const FilePath& path, ObjectIdentifier* identifier )
	: ArchiveWriter( path, identifier )
{
}

ArchiveWriterBson::ArchiveWriterBson( Stream *stream, ObjectIdentifier* identifier )
	: ArchiveWriter( identifier )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
}

ArchiveType ArchiveWriterBson::GetType() const
{
	return ArchiveTypes::Bson;
}

void ArchiveWriterBson::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Opening file '%s'\n"), m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path, FileStream::MODE_WRITE );
	m_Stream.Reset( stream );
}

void ArchiveWriterBson::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close();
	m_Stream.Release();
}

void ArchiveWriterBson::Write( Object* object )
{
	PERSIST_SCOPE_TIMER( ("Reflect - Bson Write") );

	// notify starting
	ArchiveStatus info( *this, ArchiveStates::Starting );
	e_Status.Raise( info );

	// the master object
	m_Objects.Push( object );

	bson b[1];
	bson_init( b );
	HELIUM_VERIFY( BSON_OK == bson_append_start_array( b, "BSON" ) );

	// objects can get changed during this iteration (in Identify), so use indices
	for ( size_t index = 0; index < m_Objects.GetSize(); ++index )
	{
		Object* object = m_Objects.GetElement( index );
		const MetaClass* objectClass = object->GetMetaClass();

		char num[16];
		Helium::StringPrint( num, "%d", index );
		HELIUM_VERIFY( BSON_OK == bson_append_start_object( b, num ) );
		SerializeInstance( b, objectClass->m_Name, object, objectClass, object );
		HELIUM_VERIFY( BSON_OK == bson_append_finish_object( b ) );

		info.m_State = ArchiveStates::ObjectProcessed;
		info.m_Progress = (int)(((float)(index) / (float)m_Objects.GetSize()) * 100.0f);
		e_Status.Raise( info );
	}

	bson_append_finish_array( b );

	// notify completion of last object processed
	info.m_State = ArchiveStates::ObjectProcessed;
	info.m_Progress = 100;
	e_Status.Raise( info );

	HELIUM_VERIFY( BSON_OK == bson_finish( b ) );
	m_Stream->Write( bson_data( b ), bson_size( b ), 1 );
	bson_destroy( b );

	// do cleanup
	m_Stream->Flush();

	// notify completion
	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveWriterBson::SerializeInstance( bson* b, const char* name, void* instance, const MetaStruct* structure, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print( TXT( "Serializing %s\n" ), structure->m_Name );
#endif

#pragma TODO("Declare a max depth for inheritance to save heap allocs -geoff")
	DynamicArray< const MetaStruct* > bases;
	for ( const MetaStruct* current = structure; current != NULL; current = current->m_Base )
	{
		bases.Push( current );
	}

#pragma TODO("Declare a max count for fields to save heap allocs -geoff")
	DynamicArray< const Field* > fields;
	while ( !bases.IsEmpty() )
	{
		const MetaStruct* current = bases.Pop();
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

	HELIUM_VERIFY( BSON_OK == bson_append_start_object( b, name ) );
	object->PreSerialize( NULL );

	DynamicArray< const Field* >::ConstIterator itr = fields.Begin();
	DynamicArray< const Field* >::ConstIterator end = fields.End();
	for ( ; itr != end; ++itr )
	{
		const Field* field = *itr;

		object->PreSerialize( field );

		SerializeField( b, instance, field, object );

		object->PostSerialize( field );
	}

	object->PostSerialize( NULL );
	HELIUM_VERIFY( BSON_OK == bson_append_finish_object( b ) );
}

void ArchiveWriterBson::SerializeField( bson* b, void* instance, const Field* field, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Serializing field %s\n"), field->m_Name);
#endif

	if ( field->m_Count > 1 )
	{
		HELIUM_VERIFY( BSON_OK == bson_append_start_array( b, field->m_Name ) );

		for ( uint32_t i=0; i<field->m_Count; ++i )
		{
			char num[16];
			Helium::StringPrint( num, "%d", i );
			SerializeTranslator( b, num, Pointer ( field, instance, object, i ), field->m_Translator, field, object );
		}

		HELIUM_VERIFY( BSON_OK == bson_append_finish_array( b ) );
	}
	else
	{
		SerializeTranslator( b, field->m_Name, Pointer ( field, instance, object ), field->m_Translator, field, object );
	}
}

void ArchiveWriterBson::SerializeTranslator( bson* b, const char* name, Pointer pointer, Translator* translator, const Field* field, Object* object )
{
	switch ( translator->GetReflectionType() )
	{
	case MetaIds::ScalarTranslator:
	case MetaIds::EnumerationTranslator:
	case MetaIds::PointerTranslator:
	case MetaIds::TypeTranslator:
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			switch ( scalar->m_Type )
			{
			case ScalarTypes::Boolean:
				HELIUM_VERIFY( BSON_OK == bson_append_bool( b, name, pointer.As<bool>() ) );
				break;

			case ScalarTypes::Unsigned8:
				HELIUM_VERIFY( BSON_OK == bson_append_int( b, name, pointer.As<uint8_t>() ) );
				break;

			case ScalarTypes::Unsigned16:
				HELIUM_VERIFY( BSON_OK == bson_append_int( b, name, pointer.As<uint16_t>() ) );
				break;

			case ScalarTypes::Unsigned32:
				HELIUM_VERIFY( BSON_OK == bson_append_int( b, name, pointer.As<uint32_t>() ) );
				break;

			case ScalarTypes::Unsigned64:
				HELIUM_VERIFY( BSON_OK == bson_append_long( b, name, pointer.As<int64_t>() ) ); // uint64_t isn't supported, just hope for the best
				break;

			case ScalarTypes::Signed8:
				HELIUM_VERIFY( BSON_OK == bson_append_int( b, name, pointer.As<int8_t>() ) );
				break;

			case ScalarTypes::Signed16:
				HELIUM_VERIFY( BSON_OK == bson_append_int( b, name, pointer.As<int16_t>() ) );
				break;

			case ScalarTypes::Signed32:
				HELIUM_VERIFY( BSON_OK == bson_append_int( b, name, pointer.As<int32_t>() ) );
				break;

			case ScalarTypes::Signed64:
				HELIUM_VERIFY( BSON_OK == bson_append_long( b, name, pointer.As<int64_t>() ) );
				break;

			case ScalarTypes::Float32:
				HELIUM_VERIFY( BSON_OK == bson_append_double( b, name, pointer.As<float32_t>() ) );
				break;

			case ScalarTypes::Float64:
				HELIUM_VERIFY( BSON_OK == bson_append_double( b, name, pointer.As<float64_t>() ) );
				break;

			case ScalarTypes::String:
				String str;
				scalar->Print( pointer, str, this );
				HELIUM_VERIFY( BSON_OK == bson_append_string( b, name, str.GetData() ) );
				break;
			}
			break;
		}

	case MetaIds::StructureTranslator:
		{
			StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
			SerializeInstance( b, name, pointer.m_Address, structure->GetMetaStruct(), object );
			break;
		}

	case MetaIds::SetTranslator:
		{
			SetTranslator* set = static_cast< SetTranslator* >( translator );

			Translator* itemTranslator = set->GetItemTranslator();
			DynamicArray< Pointer > items;
			set->GetItems( pointer, items );

			HELIUM_VERIFY( BSON_OK == bson_append_start_array( b, name ) );

			uint32_t index = 0;
			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr, ++index )
			{
				char num[16];
				Helium::StringPrint( num, "%d", index );
				SerializeTranslator( b, num, *itr, itemTranslator, field, object );
			}

			HELIUM_VERIFY( BSON_OK == bson_append_finish_array( b ) );
			break;
		}

	case MetaIds::SequenceTranslator:
		{
			SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );

			Translator* itemTranslator = sequence->GetItemTranslator();
			DynamicArray< Pointer > items;
			sequence->GetItems( pointer, items );

			HELIUM_VERIFY( BSON_OK == bson_append_start_array( b, name ) );

			uint32_t index = 0;
			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr, ++index )
			{
				char num[16];
				Helium::StringPrint( num, "%d", index );
				SerializeTranslator( b, num, *itr, itemTranslator, field, object );
			}

			HELIUM_VERIFY( BSON_OK == bson_append_finish_array( b ) );
			break;
		}

	case MetaIds::AssociationTranslator:
		{
			AssociationTranslator* association = static_cast< AssociationTranslator* >( translator );

			ScalarTranslator* keyTranslator = association->GetKeyTranslator();
			Translator* valueTranslator = association->GetValueTranslator();
			DynamicArray< Pointer > keys, values;
			association->GetItems( pointer, keys, values );

			HELIUM_VERIFY( BSON_OK == bson_append_start_object( b, name ) );

			for ( DynamicArray< Pointer >::Iterator keyItr = keys.Begin(), valueItr = values.Begin(), keyEnd = keys.End(), valueEnd = values.End();
				keyItr != keyEnd && valueItr != valueEnd;
				++keyItr, ++valueItr )
			{
				String name;
				keyTranslator->Print( *keyItr, name, m_Identifier );
				SerializeTranslator( b, name.GetData(), *valueItr, valueTranslator, field, object );
			}

			HELIUM_VERIFY( BSON_OK == bson_append_finish_object( b ) );
			break;
		}

	default:
		// Unhandled reflection type in ArchiveWriterBson::SerializeTranslator
		HELIUM_ASSERT_FALSE();
	}
}

void ArchiveWriterBson::ToStream( Object* object, Stream& stream, ObjectIdentifier* identifier, uint32_t flags )
{
	ArchiveWriterBson archive ( &stream, identifier );
	archive.m_Flags = flags;
	archive.Write( object );
	archive.Close();
}

ArchiveReaderBson::ArchiveReaderBson( const FilePath& path, ObjectResolver* resolver )
	: ArchiveReader( path, resolver )
	, m_Stream( NULL )
	, m_Size( 0 )
{

}

ArchiveReaderBson::ArchiveReaderBson( Stream *stream, ObjectResolver* resolver )
	: ArchiveReader( resolver )
	, m_Stream( NULL )
	, m_Size( 0 )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
}

ArchiveType ArchiveReaderBson::GetType() const
{
	return ArchiveTypes::Bson;
}

void ArchiveReaderBson::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Opening file '%s'\n"), m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path, FileStream::MODE_READ );
	m_Stream.Reset( stream );
}

void ArchiveReaderBson::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close();
	m_Stream.Release();
}

void ArchiveReaderBson::Read( Reflect::ObjectPtr& object )
{
	PERSIST_SCOPE_TIMER( ("Reflect - Bson Read") );

	Start();

	bson_iterator i[1];
	bson_iterator_init( i, m_Bson );

	if ( bson_iterator_type( i ) == BSON_ARRAY )
	{
		bson_iterator_subiterator( i, m_Next );

		while ( bson_iterator_more( m_Next ) )
		{
			ObjectPtr object;
			ReadNext( object );

			ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
			info.m_Progress = (int)(((float)(m_Stream->Tell()) / (float)m_Size) * 100.0f);
			e_Status.Raise( info );
			m_Abort |= info.m_Abort;
			if ( m_Abort )
			{
				break;
			}
		}
	}

	Resolve();

	if ( !m_Objects.IsEmpty() )
	{
		object = m_Objects.GetFirst();
	}

	bson_destroy( m_Bson );
}

void Helium::Persist::ArchiveReaderBson::Start()
{
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

	// read entire contents
	m_Buffer.Resize( static_cast< size_t >( m_Size + 1 ) );
	m_Stream->Read( m_Buffer.GetData(),  static_cast< size_t >( m_Size ), 1 );
	m_Buffer[ static_cast< size_t >( m_Size ) ] = '\0';

	HELIUM_VERIFY( BSON_OK == bson_init_finished_data( m_Bson, reinterpret_cast< char* >( m_Buffer.GetData() ), false ) );
}

bool Helium::Persist::ArchiveReaderBson::ReadNext( Reflect::ObjectPtr& object )
{
	bool success = false;

	if ( !bson_iterator_more( m_Next ) || bson_iterator_type( m_Next ) != BSON_OBJECT )
	{
		return false;
	}

	bson_iterator i[1];
	bson_iterator_subiterator( m_Next, i );

	if ( bson_iterator_type( i ) == BSON_OBJECT )
	{
		const char* key = bson_iterator_key( i );

		uint32_t objectClassCrc = 0;
		if ( key )
		{
			objectClassCrc = Helium::Crc32( key );
		}

		const MetaClass* objectClass = NULL;
		if ( objectClassCrc != 0 )
		{
			objectClass = Registry::GetInstance()->GetMetaClass( objectClassCrc );
		}
			
		if ( !object && objectClass )
		{
			object = objectClass->m_Creator();
		}

		if ( object.ReferencesObject() )
		{
			success = true;
			DeserializeInstance( i, object, object->GetMetaClass(), object );

			m_Objects.Push( object );
		}
	}

	bson_iterator_next( m_Next );

	return success;
}

void Helium::Persist::ArchiveReaderBson::Resolve()
{
	ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
	info.m_Progress = 100;
	e_Status.Raise( info );

	// finish linking objects (unless we have a custom handler)
	for ( DynamicArray< Fixup >::ConstIterator itr = m_Fixups.Begin(), end = m_Fixups.End(); itr != end; ++itr )
	{
		ArchiveReader::Resolve( itr->m_Identity, itr->m_Pointer, itr->m_PointerClass );
	}

	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveReaderBson::DeserializeInstance( bson_iterator* i, void* instance, const MetaStruct* structure, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Deserializing %s\n"), structure->m_Name);
#endif
	object->PreDeserialize( NULL );

	if ( bson_iterator_type( i ) == BSON_OBJECT )
	{
		bson_iterator obj[1];
		bson_iterator_subiterator( i, obj );

		while( bson_iterator_next( obj ) )
		{
			const char* key = bson_iterator_key( obj );

			uint32_t fieldCrc = 0;
			if ( key )
			{
				fieldCrc = Helium::Crc32( key );
			}

			const Field* field = structure->FindFieldByName( fieldCrc );
			if ( field )
			{
				object->PreDeserialize( field );

				DeserializeField( obj, instance, field, object );

				object->PostDeserialize( field );
			}
			else
			{
				if ( key )
				{
					HELIUM_TRACE(
						TraceLevels::Warning, 
						"ArchiveReaderBson::DeserializeInstance - Could not find field '%s' (crc=)\n", 
						key, 
						fieldCrc);
				}
				else
				{
					HELIUM_TRACE(
						TraceLevels::Warning, 
						"ArchiveReaderBson::DeserializeInstance - Could not find field (crc=)\n", 
						fieldCrc);
				}
				
			}
		}
	}

	object->PostDeserialize( NULL );
}

void ArchiveReaderBson::DeserializeField( bson_iterator* i, void* instance, const Field* field, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Deserializing field %s\n"), field->m_Name);
#endif
	
	if ( field->m_Count > 1 )
	{
		if ( bson_iterator_type( i ) == BSON_ARRAY )
		{
			bson_iterator elem[1];
			bson_iterator_subiterator( i, elem );

			uint32_t index = 0;
			while( bson_iterator_next( elem ) )
			{
				if ( index++ < field->m_Count )
				{
					DeserializeTranslator( elem, Pointer ( field, instance, object, index ), field->m_Translator, field, object );
				}
			}
		}
		else
		{
			DeserializeTranslator( i, Pointer ( field, instance, object, 0 ), field->m_Translator, field, object );
		}
	}
	else
	{
		DeserializeTranslator( i, Pointer ( field, instance, object ), field->m_Translator, field, object );
	}
}

void ArchiveReaderBson::DeserializeTranslator( bson_iterator* i, Pointer pointer, Translator* translator, const Field* field, Object* object )
{
	switch ( bson_iterator_type( i ) )
	{
	case BSON_BOOL:
		{
			if ( translator->GetReflectionType() == MetaIds::ScalarTranslator )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				if ( scalar->m_Type == ScalarTypes::Boolean )
				{
					pointer.As<bool>() = bson_iterator_bool( i ) != 0;
				}
			}
			break;
		}

	case BSON_INT:
		{
			if ( translator->GetReflectionType() == MetaIds::ScalarTranslator )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				bool clamp = true;
				switch ( scalar->m_Type )
				{
				case ScalarTypes::Boolean:
				case ScalarTypes::String:
					break;
				
				case ScalarTypes::Unsigned8:
					RangeCastInteger( bson_iterator_int( i ), pointer.As<uint8_t>(), clamp );
					break;

				case ScalarTypes::Unsigned16:
					RangeCastInteger( bson_iterator_int( i ), pointer.As<uint16_t>(), clamp );
					break;

				case ScalarTypes::Unsigned32:
					RangeCastInteger( bson_iterator_int( i ), pointer.As<uint32_t>(), clamp );
					break;

				case ScalarTypes::Unsigned64:
					RangeCastInteger( bson_iterator_int( i ), pointer.As<uint64_t>(), clamp );
					break;

				case ScalarTypes::Signed8:
					RangeCastInteger( bson_iterator_int( i ), pointer.As<int8_t>(), clamp );
					break;

				case ScalarTypes::Signed16:
					RangeCastInteger( bson_iterator_int( i ), pointer.As<int16_t>(), clamp );
					break;

				case ScalarTypes::Signed32:
					RangeCastInteger( bson_iterator_int( i ), pointer.As<int32_t>(), clamp );
					break;

				case ScalarTypes::Signed64:
					RangeCastInteger( bson_iterator_int( i ), pointer.As<int64_t>(), clamp );
					break;

				case ScalarTypes::Float32:
					RangeCastFloat( bson_iterator_int( i ), pointer.As<float32_t>(), clamp );
					break;

				case ScalarTypes::Float64:
					RangeCastFloat( bson_iterator_int( i ), pointer.As<float64_t>(), clamp );
					break;
				}
			}
			break;
		}

	case BSON_LONG:
		{
			if ( translator->GetReflectionType() == MetaIds::ScalarTranslator )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				bool clamp = true;
				switch ( scalar->m_Type )
				{
				case ScalarTypes::Boolean:
				case ScalarTypes::String:
					break;
				
				case ScalarTypes::Unsigned8:
					RangeCastInteger( bson_iterator_long( i ), pointer.As<uint8_t>(), clamp );
					break;

				case ScalarTypes::Unsigned16:
					RangeCastInteger( bson_iterator_long( i ), pointer.As<uint16_t>(), clamp );
					break;

				case ScalarTypes::Unsigned32:
					RangeCastInteger( bson_iterator_long( i ), pointer.As<uint32_t>(), clamp );
					break;

				case ScalarTypes::Unsigned64:
					RangeCastInteger( bson_iterator_long( i ), pointer.As<uint64_t>(), clamp );
					break;

				case ScalarTypes::Signed8:
					RangeCastInteger( bson_iterator_long( i ), pointer.As<int8_t>(), clamp );
					break;

				case ScalarTypes::Signed16:
					RangeCastInteger( bson_iterator_long( i ), pointer.As<int16_t>(), clamp );
					break;

				case ScalarTypes::Signed32:
					RangeCastInteger( bson_iterator_long( i ), pointer.As<int32_t>(), clamp );
					break;

				case ScalarTypes::Signed64:
					RangeCastInteger( bson_iterator_long( i ), pointer.As<int64_t>(), clamp );
					break;

				case ScalarTypes::Float32:
					RangeCastFloat( bson_iterator_long( i ), pointer.As<float32_t>(), clamp );
					break;

				case ScalarTypes::Float64:
					RangeCastFloat( bson_iterator_long( i ), pointer.As<float64_t>(), clamp );
					break;
				}
			}
			break;
		}

	case BSON_DOUBLE:
		{
			if ( translator->GetReflectionType() == MetaIds::ScalarTranslator )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				bool clamp = true;
				switch ( scalar->m_Type )
				{
				case ScalarTypes::Boolean:
				case ScalarTypes::String:
					break;
				
				case ScalarTypes::Unsigned8:
					RangeCastInteger( bson_iterator_double( i ), pointer.As<uint8_t>(), clamp );
					break;

				case ScalarTypes::Unsigned16:
					RangeCastInteger( bson_iterator_double( i ), pointer.As<uint16_t>(), clamp );
					break;

				case ScalarTypes::Unsigned32:
					RangeCastInteger( bson_iterator_double( i ), pointer.As<uint32_t>(), clamp );
					break;

				case ScalarTypes::Unsigned64:
					RangeCastInteger( bson_iterator_double( i ), pointer.As<uint64_t>(), clamp );
					break;

				case ScalarTypes::Signed8:
					RangeCastInteger( bson_iterator_double( i ), pointer.As<int8_t>(), clamp );
					break;

				case ScalarTypes::Signed16:
					RangeCastInteger( bson_iterator_double( i ), pointer.As<int16_t>(), clamp );
					break;

				case ScalarTypes::Signed32:
					RangeCastInteger( bson_iterator_double( i ), pointer.As<int32_t>(), clamp );
					break;

				case ScalarTypes::Signed64:
					RangeCastInteger( bson_iterator_double( i ), pointer.As<int64_t>(), clamp );
					break;

				case ScalarTypes::Float32:
					RangeCastFloat( bson_iterator_double( i ), pointer.As<float32_t>(), clamp );
					break;

				case ScalarTypes::Float64:
					RangeCastFloat( bson_iterator_double( i ), pointer.As<float64_t>(), clamp );
					break;
				}
			}
			break;
		}

	case BSON_STRING:
		{
			if ( translator->HasReflectionType( MetaIds::ScalarTranslator ) )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				if ( scalar->m_Type == ScalarTypes::String )
				{
					String str ( bson_iterator_string( i ) );
					scalar->Parse( str, pointer, this, m_Flags | ArchiveFlags::Notify ? true : false );
				}
			}
			break;
		}

	case BSON_ARRAY:
		{
			if ( translator->GetReflectionType() == MetaIds::SetTranslator )
			{
				SetTranslator* set = static_cast< SetTranslator* >( translator );
				Translator* itemTranslator = set->GetItemTranslator();

				bson_iterator elem[1];
				bson_iterator_subiterator( i, elem );

				while( bson_iterator_next( elem ) )
				{
					Variable item ( itemTranslator );
					DeserializeTranslator( elem, item, itemTranslator, field, object );
					set->InsertItem( pointer, item );
				}
			}
			else if ( translator->GetReflectionType() == MetaIds::SequenceTranslator )
			{
				SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );
				Translator* itemTranslator = sequence->GetItemTranslator();

				bson_iterator elem[1];
				bson_iterator_subiterator( i, elem );

				while( bson_iterator_next( elem ) )
				{
					Variable item ( itemTranslator );
					DeserializeTranslator( elem, item, itemTranslator, field, object );
					sequence->Insert( pointer, sequence->GetLength( pointer ), item );
				}
			}
			break;
		}

	case BSON_OBJECT:
		{
			if ( translator->GetReflectionType() == MetaIds::StructureTranslator )
			{
				StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
				DeserializeInstance( i, pointer.m_Address,  structure->GetMetaStruct(), object );
			}
			else if ( translator->GetReflectionType() == MetaIds::AssociationTranslator )
			{
				AssociationTranslator* assocation = static_cast< AssociationTranslator* >( translator );
				ScalarTranslator* keyTranslator = assocation->GetKeyTranslator();
				Translator* valueTranslator = assocation->GetValueTranslator();
				Variable keyVariable ( keyTranslator );
				Variable valueVariable ( valueTranslator );

				bson_iterator elem[1];
				bson_iterator_subiterator( i, elem );

				while( bson_iterator_next( elem ) )
				{
					String key ( bson_iterator_key( elem ) );
					keyTranslator->Parse( key, keyVariable, m_Resolver );
					DeserializeTranslator( elem, valueVariable, valueTranslator, field, object );
					assocation->SetItem( pointer, keyVariable, valueVariable );
				}
			}
			break;
		}

	default:
		break;
	}
}

ObjectPtr ArchiveReaderBson::FromStream( Stream& stream, ObjectResolver* resolver, uint32_t flags )
{
	ArchiveReaderBson archive( &stream, resolver );
	archive.m_Flags = flags;
	ObjectPtr object;
	archive.Read( object );
	archive.Close();
	return object;
}
