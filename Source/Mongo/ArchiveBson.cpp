#include "Precompile.h"
#include "Mongo/ArchiveBson.h"

#include "Foundation/Endian.h"
#include "Foundation/FileStream.h"
#include "Foundation/Numeric.h"

#include "Reflect/Object.h"
#include "Reflect/MetaStruct.h"
#include "Reflect/Registry.h"
#include "Reflect/TranslatorDeduction.h"

#include <time.h>

HELIUM_DEFINE_BASE_STRUCT( Helium::Persist::BsonDateTime );
HELIUM_DEFINE_BASE_STRUCT( Helium::Persist::BsonObjectId );
HELIUM_DEFINE_BASE_STRUCT( Helium::Persist::BsonGeoPoint );
HELIUM_DEFINE_BASE_STRUCT( Helium::Persist::BsonGeoLineString );
HELIUM_DEFINE_BASE_STRUCT( Helium::Persist::BsonGeoPolygon );

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;

BsonObjectId BsonObjectId::Null;

void BsonDateTime::PopulateMetaType( Reflect::MetaStruct& type )
{
	type.AddField( &BsonDateTime::millis, "millis" );
}

BsonDateTime BsonDateTime::Now()
{
    time_t t;
    time( &t );

    BsonDateTime dt;
    dt.millis = t * 1000;
    return dt;
}

void BsonObjectId::IntoString( String& str ) const
{
	char chars[ sizeof(bson_oid_t) * 2 + 1 ] = { 0 };
	bson_oid_to_string( (bson_oid_t*)this, chars );
	str = chars;
}

void BsonObjectId::FromString( const String& str )
{
	bson_oid_init_from_string( (bson_oid_t*)this, str.GetData() );
}

void BsonObjectId::PopulateMetaType( Reflect::MetaStruct& type )
{
	type.AddField( &BsonObjectId::bytes, "bytes" );
}

void BsonGeoPoint::PopulateMetaType( Helium::Reflect::MetaStruct& type )
{
	type.AddField( &BsonGeoPoint::type, "type", Reflect::FieldFlags::Force );
	type.AddField( &BsonGeoPoint::coordinates, "coordinates" );
}

void BsonGeoLineString::PopulateMetaType( Helium::Reflect::MetaStruct& type )
{
	type.AddField( &BsonGeoLineString::type, "type", Reflect::FieldFlags::Force );
	type.AddField( &BsonGeoLineString::coordinates, "coordinates" );
}

void BsonGeoPolygon::PopulateMetaType( Helium::Reflect::MetaStruct& type )
{
	type.AddField( &BsonGeoPolygon::type, "type", Reflect::FieldFlags::Force );
	type.AddField( &BsonGeoPolygon::coordinates, "coordinates" );
}

void ArchiveWriterBson::Startup()
{
	Register( "bson", &AllocateWriter );
}

void ArchiveWriterBson::Shutdown()
{
	Unregister( "bson" );
}

SmartPtr< ArchiveWriter > ArchiveWriterBson::AllocateWriter( const FilePath& path, Reflect::ObjectIdentifier* identifier )
{
	return new ArchiveWriterBson( path, identifier );
}

void ArchiveWriterBson::WriteToStream( const ObjectPtr& object, Stream& stream, ObjectIdentifier* identifier, uint32_t flags )
{
	ArchiveWriterBson archive ( &stream, identifier, flags );
	archive.Write( &object, 1 );
	archive.Close();
}

void ArchiveWriterBson::WriteToStream( const ObjectPtr* objects, size_t count, Stream& stream, ObjectIdentifier* identifier, uint32_t flags )
{
	ArchiveWriterBson archive ( &stream, identifier, flags );
	archive.Write( objects, count );
	archive.Close();
}

void ArchiveWriterBson::WriteToBson( const ObjectPtr& object, bson_t* b, const char* name, Reflect::ObjectIdentifier* identifier, uint32_t flags )
{
	ArchiveWriterBson archive ( NULL, identifier, flags );
	archive.SerializeInstance( b, name, object, object->GetMetaClass(), object );
}

ArchiveWriterBson::ArchiveWriterBson( const FilePath& path, ObjectIdentifier* identifier, uint32_t flags )
	: ArchiveWriter( path, identifier, flags )
{
}

ArchiveWriterBson::ArchiveWriterBson( Stream *stream, ObjectIdentifier* identifier, uint32_t flags )
	: ArchiveWriter( identifier, flags )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
}

void ArchiveWriterBson::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Opening file '%s'\n", m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path.Data(), FileStream::MODE_WRITE );
	m_Stream.Reset( stream );
}

void ArchiveWriterBson::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close();
}

void ArchiveWriterBson::Write( const ObjectPtr* objects, size_t count )
{
	HELIUM_PERSIST_SCOPE_TIMER( "Reflect - Bson Write" );

	// notify starting
	ArchiveStatus info( *this, ArchiveStates::Starting );
	e_Status.Raise( info );

	// the master object
	m_Objects.AddArray( objects, count );

	bson_t bson;
	bson_init( &bson );

	try
	{
		bson_t childArray;
		HELIUM_VERIFY( bson_append_array_begin( &bson, "objects", -1, &childArray) );

		// objects can get changed during this iteration (in Identify), so use indices
		for ( size_t index = 0; index < m_Objects.GetSize(); ++index )
		{
			Object* object = m_Objects.GetElement( index );
			const MetaClass* objectClass = object->GetMetaClass();

			char num[16];
			Helium::StringPrint( num, "%d", index );

			bson_t objectDocument;
			HELIUM_VERIFY( bson_append_document_begin( &childArray, num, -1, &objectDocument ) );
			SerializeInstance( &objectDocument, objectClass->m_Name, object, objectClass, object );
			HELIUM_VERIFY( bson_append_document_end( &childArray, &objectDocument ) );

			info.m_State = ArchiveStates::ObjectProcessed;
			info.m_Progress = (int)(((float)(index) / (float)m_Objects.GetSize()) * 100.0f);
			e_Status.Raise( info );
		}

		HELIUM_VERIFY( bson_append_array_end( &bson, &childArray ) );

		// notify completion of last object processed
		info.m_State = ArchiveStates::ObjectProcessed;
		info.m_Progress = 100;
		e_Status.Raise( info );

		m_Stream->Write( bson_get_data( &bson ), bson.len, 1 );
	}
	catch( ... )
	{
		bson_destroy( &bson );
		throw;
	}

	bson_destroy( &bson );

	// do cleanup
	m_Stream->Flush();

	// notify completion
	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveWriterBson::SerializeInstance( bson_t* bson, const char* name, void* instance, const MetaStruct* structure, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Serializing %s\n", structure->m_Name );
#endif

	// TODO: Declare a max depth for inheritance to save heap allocs -geoff
	DynamicArray< const MetaStruct* > bases;
	for ( const MetaStruct* current = structure; current != NULL; current = current->m_Base )
	{
		bases.Push( current );
	}

	// TODO: Declare a max count for fields to save heap allocs -geoff
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

	bson_t* parent = bson;
	bson_t childDocument;

	if ( name )
	{
		HELIUM_VERIFY( bson_append_document_begin( bson, name, -1, &childDocument ) );
		bson = &childDocument;
	}

	object->PreSerialize( NULL );

	DynamicArray< const Field* >::ConstIterator itr = fields.Begin();
	DynamicArray< const Field* >::ConstIterator end = fields.End();
	for ( ; itr != end; ++itr )
	{
		const Field* field = *itr;

		object->PreSerialize( field );

		SerializeField( bson, instance, field, object );

		object->PostSerialize( field );
	}

	object->PostSerialize( NULL );

	if ( name )
	{
		HELIUM_VERIFY( bson_append_document_end( parent, &childDocument ) );
		bson = parent;
	}
}

void ArchiveWriterBson::SerializeField( bson_t* bson, void* instance, const Field* field, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Serializing field %s\n", field->m_Name);
#endif

	if ( field->m_Count > 1 )
	{
		bson_t childArray;
		HELIUM_VERIFY( bson_append_array_begin( bson, field->m_Name, -1, &childArray ) );

		for ( uint32_t i=0; i<field->m_Count; ++i )
		{
			char num[16];
			Helium::StringPrint( num, "%d", i );
			SerializeTranslator( &childArray, num, Pointer ( field, instance, object, i ), field->m_Translator, field, object );
		}

		HELIUM_VERIFY( bson_append_array_end( bson, &childArray ) );
	}
	else
	{
		SerializeTranslator( bson, field->m_Name, Pointer ( field, instance, object ), field->m_Translator, field, object );
	}
}

void ArchiveWriterBson::SerializeTranslator( bson_t* bson, const char* name, Pointer pointer, Translator* translator, const Field* field, Object* object )
{
	switch ( translator->GetMetaId() )
	{
	case MetaIds::ScalarTranslator:
	case MetaIds::SimpleTranslator:
	case MetaIds::EnumerationTranslator:
	case MetaIds::PointerTranslator:
	case MetaIds::TypeTranslator:
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			switch ( scalar->m_Type )
			{
			case ScalarTypes::Boolean:
				HELIUM_VERIFY( bson_append_bool( bson, name, -1, pointer.As<bool>() ) );
				break;

			case ScalarTypes::Unsigned8:
				HELIUM_VERIFY( bson_append_int32( bson, name, -1, pointer.As<uint8_t>() ) );
				break;

			case ScalarTypes::Unsigned16:
				HELIUM_VERIFY( bson_append_int32( bson, name, -1, pointer.As<uint16_t>() ) );
				break;

			case ScalarTypes::Unsigned32:
				HELIUM_VERIFY( bson_append_int32( bson, name, -1, pointer.As<uint32_t>() ) );
				break;

			case ScalarTypes::Unsigned64:
				HELIUM_VERIFY( bson_append_int64( bson, name, -1, pointer.As<int64_t>() ) ); // uint64_t isn't supported, just hope for the best
				break;

			case ScalarTypes::Signed8:
				HELIUM_VERIFY( bson_append_int32( bson, name, -1, pointer.As<int8_t>() ) );
				break;

			case ScalarTypes::Signed16:
				HELIUM_VERIFY( bson_append_int32( bson, name, -1, pointer.As<int16_t>() ) );
				break;

			case ScalarTypes::Signed32:
				HELIUM_VERIFY( bson_append_int32( bson, name, -1, pointer.As<int32_t>() ) );
				break;

			case ScalarTypes::Signed64:
				HELIUM_VERIFY( bson_append_int64( bson, name, -1, pointer.As<int64_t>() ) );
				break;

			case ScalarTypes::Float32:
				HELIUM_VERIFY( bson_append_double( bson, name, -1, pointer.As<float32_t>() ) );
				break;

			case ScalarTypes::Float64:
				HELIUM_VERIFY( bson_append_double( bson, name, -1, pointer.As<float64_t>() ) );
				break;

			case ScalarTypes::String:
				String str;
				scalar->Print( pointer, str, this );
				HELIUM_VERIFY( bson_append_utf8( bson, name, -1, str.GetData(), (int)str.GetSize() ) );
				break;
			}
			break;
		}

	case MetaIds::StructureTranslator:
		{
			StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
			const MetaStruct* metaStruct = structure->GetMetaStruct();
			if ( metaStruct == Reflect::GetMetaStruct< BsonDateTime >() )
			{
				bson_append_date_time( bson, name, -1, pointer.As< BsonDateTime >().millis );
			}
			else if ( metaStruct == Reflect::GetMetaStruct< BsonObjectId >() )
			{
				bson_oid_t oid;
				MemoryCopy( oid.bytes, pointer.As< BsonObjectId >().bytes, sizeof( bson_oid_t ) );
				bson_append_oid( bson, name, -1, &oid );
			}
			else
			{
				SerializeInstance( bson, name, pointer.m_Address, structure->GetMetaStruct(), object );
			}
			break;
		}

	case MetaIds::SetTranslator:
		{
			SetTranslator* set = static_cast< SetTranslator* >( translator );

			Translator* itemTranslator = set->GetItemTranslator();
			DynamicArray< Pointer > items;
			set->GetItems( pointer, items );

			bson_t childArray;
			HELIUM_VERIFY( bson_append_array_begin( bson, name, -1, &childArray ) );

			uint32_t index = 0;
			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr, ++index )
			{
				char num[16];
				Helium::StringPrint( num, "%d", index );
				SerializeTranslator( &childArray, num, *itr, itemTranslator, field, object );
			}

			HELIUM_VERIFY( bson_append_array_end( bson, &childArray ) );
			break;
		}

	case MetaIds::SequenceTranslator:
		{
			SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );

			Translator* itemTranslator = sequence->GetItemTranslator();
			DynamicArray< Pointer > items;
			sequence->GetItems( pointer, items );

			bson_t childArray;
			HELIUM_VERIFY( bson_append_array_begin( bson, name, -1, &childArray ) );

			uint32_t index = 0;
			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr, ++index )
			{
				char num[16];
				Helium::StringPrint( num, "%d", index );
				SerializeTranslator( &childArray, num, *itr, itemTranslator, field, object );
			}

			HELIUM_VERIFY( bson_append_array_end( bson, &childArray ) );
			break;
		}

	case MetaIds::AssociationTranslator:
		{
			AssociationTranslator* association = static_cast< AssociationTranslator* >( translator );

			ScalarTranslator* keyTranslator = association->GetKeyTranslator();
			Translator* valueTranslator = association->GetValueTranslator();
			DynamicArray< Pointer > keys, values;
			association->GetItems( pointer, keys, values );

			bson_t childDocument;
			HELIUM_VERIFY( bson_append_document_begin( bson, name, -1, &childDocument ) );

			for ( DynamicArray< Pointer >::Iterator keyItr = keys.Begin(), valueItr = values.Begin(), keyEnd = keys.End(), valueEnd = values.End();
				keyItr != keyEnd && valueItr != valueEnd;
				++keyItr, ++valueItr )
			{
				String name;
				keyTranslator->Print( *keyItr, name, m_Identifier );
				SerializeTranslator( &childDocument, name.GetData(), *valueItr, valueTranslator, field, object );
			}

			HELIUM_VERIFY( bson_append_document_end( bson, &childDocument ) );
			break;
		}

	default:
		// Unhandled reflection type in ArchiveWriterBson::SerializeTranslator
		HELIUM_BREAK();
	}
}

void ArchiveReaderBson::Startup()
{
	Register( "bson", &AllocateReader );
}

void ArchiveReaderBson::Shutdown()
{
	Unregister( "bson" );
}

SmartPtr< ArchiveReader > ArchiveReaderBson::AllocateReader( const FilePath& path, Reflect::ObjectResolver* resolver )
{
	return new ArchiveReaderBson( path, resolver );
}

void ArchiveReaderBson::ReadFromStream( Stream& stream, ObjectPtr& object, ObjectResolver* resolver, uint32_t flags )
{
	DynamicArray< ObjectPtr > objects;
	ReadFromStream( stream, objects, resolver, flags );
	if ( !objects.IsEmpty() )
	{
		object = objects.GetFirst();
	}
}

void ArchiveReaderBson::ReadFromStream( Stream& stream, DynamicArray< ObjectPtr >& objects, ObjectResolver* resolver, uint32_t flags )
{
	ArchiveReaderBson archive( &stream, resolver, flags );
	archive.Read( objects );
	archive.Close();
}

void ArchiveReaderBson::ReadFromBson( bson_iter_t* i, const ObjectPtr& object, ObjectResolver* resolver, uint32_t flags )
{
	ArchiveReaderBson archive( NULL, resolver, flags );
	archive.DeserializeInstance( i, object.Ptr(), object->GetMetaClass(), object );
}

ArchiveReaderBson::ArchiveReaderBson( const FilePath& path, ObjectResolver* resolver, uint32_t flags )
	: ArchiveReader( path, resolver, flags )
	, m_Stream( NULL )
	, m_Size( 0 )
{

}

ArchiveReaderBson::ArchiveReaderBson( Stream *stream, ObjectResolver* resolver, uint32_t flags )
	: ArchiveReader( resolver, flags )
	, m_Stream( NULL )
	, m_Size( 0 )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
}

void ArchiveReaderBson::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Opening file '%s'\n", m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path.Data(), FileStream::MODE_READ );
	m_Stream.Reset( stream );
}

void ArchiveReaderBson::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close();
}

void ArchiveReaderBson::Read( DynamicArray< Reflect::ObjectPtr >& objects )
{
	HELIUM_PERSIST_SCOPE_TIMER( "Reflect - Bson Read" );

	Start();

	m_Objects = objects;

	bson_iter_t iterator;
	bson_iter_init( &iterator, &m_Bson );

	if ( HELIUM_VERIFY( bson_iter_type( &iterator ) == BSON_TYPE_ARRAY ) )
	{
		bson_iter_recurse( &iterator, &m_Next );
		for ( size_t i=0; bson_iter_next( &m_Next ); ++i )
		{
			if ( i+1 > m_Objects.GetSize() )
			{
				m_Objects.Push( NULL );
			}

			ObjectPtr& object( m_Objects[i] );
			ReadNext( object, i );

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

	objects = m_Objects;

	bson_destroy( &m_Bson );
}

void ArchiveReaderBson::Start()
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
		throw Persist::StreamException( "Input stream is empty (%s)", m_Path.Data() );
	}

	// read entire contents
	m_Buffer.Resize( static_cast< size_t >( m_Size + 1 ) );
	m_Stream->Read( m_Buffer.GetData(),  static_cast< size_t >( m_Size ), 1 );
	m_Buffer[ static_cast< size_t >( m_Size ) ] = '\0';

	if ( !HELIUM_VERIFY( bson_init_static( &m_Bson, m_Buffer.GetData(), m_Size ) ) )
	{
		throw Persist::Exception( "Bson error." );
	}
}

bool ArchiveReaderBson::ReadNext( Reflect::ObjectPtr& object, size_t index )
{
	bson_iter_t iterator;
	bson_iter_recurse( &m_Next, &iterator );
	if ( HELIUM_VERIFY( bson_iter_type( &iterator ) == BSON_TYPE_DOCUMENT ) )
	{
		const char* key = bson_iter_key( &iterator );

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

		if ( !object && HELIUM_VERIFY( objectClass ) )
		{
			object = AllocateObject( objectClass, index );
		}

		if ( object.ReferencesObject() )
		{
			bson_iter_t child;
			bson_iter_recurse( &iterator, &child );
			DeserializeInstance( &child, object, object->GetMetaClass(), object );
		}
	}
	
	return bson_iter_next( &m_Next );
}

void ArchiveReaderBson::DeserializeInstance( bson_iter_t* iterator, void* instance, const MetaStruct* structure, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Deserializing %s\n", structure->m_Name);
#endif
	object->PreDeserialize( NULL );

	while( bson_iter_next( iterator ) )
	{
		const char* key = bson_iter_key( iterator );

		uint32_t fieldCrc = 0;
		if ( key )
		{
			fieldCrc = Helium::Crc32( key );
		}

		const Field* field = structure->FindFieldByName( fieldCrc );
		if ( field )
		{
			object->PreDeserialize( field );

			DeserializeField( iterator, instance, field, object );

			object->PostDeserialize( field );
		}
		else
		{
			if ( key )
			{
				HELIUM_TRACE(
					TraceLevels::Debug,
					"ArchiveReaderBson::DeserializeInstance - Could not find field '%s' (CRC-32 = %" PRIu32 ")\n",
					key,
					fieldCrc);
			}
			else
			{
				HELIUM_TRACE(
					TraceLevels::Debug,
					"ArchiveReaderBson::DeserializeInstance - Could not find field with CRC-32 %" PRIu32 ")\n",
					fieldCrc);
			}
				
		}
	}

	object->PostDeserialize( NULL );
}

void ArchiveReaderBson::DeserializeField( bson_iter_t* iterator, void* instance, const Field* field, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Deserializing field %s\n", field->m_Name);
#endif
	
	if ( field->m_Count > 1 )
	{
		if ( bson_iter_type( iterator ) == BSON_TYPE_ARRAY )
		{
			bson_iter_t child;
			bson_iter_recurse( iterator, &child );

			uint32_t index = 0;
			while( bson_iter_next( &child ) )
			{
				if ( index >= field->m_Count )
				{
					break;
				}

				DeserializeTranslator( &child, Pointer ( field, instance, object, index++ ), field->m_Translator, field, object );
			}
		}
		else
		{
			DeserializeTranslator( iterator, Pointer ( field, instance, object, 0 ), field->m_Translator, field, object );
		}
	}
	else
	{
		DeserializeTranslator( iterator, Pointer ( field, instance, object ), field->m_Translator, field, object );
	}
}

void ArchiveReaderBson::DeserializeTranslator( bson_iter_t* iterator, Pointer pointer, Translator* translator, const Field* field, Object* object )
{
	switch ( bson_iter_type( iterator ) )
	{
	case BSON_TYPE_BOOL:
		{
			if ( translator->IsA( MetaIds::ScalarTranslator) )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				if ( scalar->m_Type == ScalarTypes::Boolean )
				{
					pointer.As<bool>() = bson_iter_bool( iterator ) != 0;
				}
			}
			break;
		}

	case BSON_TYPE_INT32:
		{
			if ( translator->IsA( MetaIds::ScalarTranslator ) )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				bool clamp = true;
				switch ( scalar->m_Type )
				{
				case ScalarTypes::Boolean:
				case ScalarTypes::String:
					break;
				
				case ScalarTypes::Unsigned8:
					RangeCastInteger( bson_iter_int32( iterator ), pointer.As<uint8_t>(), clamp );
					break;

				case ScalarTypes::Unsigned16:
					RangeCastInteger( bson_iter_int32( iterator ), pointer.As<uint16_t>(), clamp );
					break;

				case ScalarTypes::Unsigned32:
					RangeCastInteger( bson_iter_int32( iterator ), pointer.As<uint32_t>(), clamp );
					break;

				case ScalarTypes::Unsigned64:
					RangeCastInteger( bson_iter_int32( iterator ), pointer.As<uint64_t>(), clamp );
					break;

				case ScalarTypes::Signed8:
					RangeCastInteger( bson_iter_int32( iterator ), pointer.As<int8_t>(), clamp );
					break;

				case ScalarTypes::Signed16:
					RangeCastInteger( bson_iter_int32( iterator ), pointer.As<int16_t>(), clamp );
					break;

				case ScalarTypes::Signed32:
					RangeCastInteger( bson_iter_int32( iterator ), pointer.As<int32_t>(), clamp );
					break;

				case ScalarTypes::Signed64:
					RangeCastInteger( bson_iter_int32( iterator ), pointer.As<int64_t>(), clamp );
					break;

				case ScalarTypes::Float32:
					RangeCastFloat( bson_iter_int32( iterator ), pointer.As<float32_t>(), clamp );
					break;

				case ScalarTypes::Float64:
					RangeCastFloat( bson_iter_int32( iterator ), pointer.As<float64_t>(), clamp );
					break;
				}
			}
			break;
		}

	case BSON_TYPE_INT64:
		{
			if ( translator->IsA( MetaIds::ScalarTranslator ) )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				bool clamp = true;
				switch ( scalar->m_Type )
				{
				case ScalarTypes::Boolean:
				case ScalarTypes::String:
					break;
				
				case ScalarTypes::Unsigned8:
					RangeCastInteger( bson_iter_int64( iterator ), pointer.As<uint8_t>(), clamp );
					break;

				case ScalarTypes::Unsigned16:
					RangeCastInteger( bson_iter_int64( iterator ), pointer.As<uint16_t>(), clamp );
					break;

				case ScalarTypes::Unsigned32:
					RangeCastInteger( bson_iter_int64( iterator ), pointer.As<uint32_t>(), clamp );
					break;

				case ScalarTypes::Unsigned64:
					RangeCastInteger( bson_iter_int64( iterator ), pointer.As<uint64_t>(), clamp );
					break;

				case ScalarTypes::Signed8:
					RangeCastInteger( bson_iter_int64( iterator ), pointer.As<int8_t>(), clamp );
					break;

				case ScalarTypes::Signed16:
					RangeCastInteger( bson_iter_int64( iterator ), pointer.As<int16_t>(), clamp );
					break;

				case ScalarTypes::Signed32:
					RangeCastInteger( bson_iter_int64( iterator ), pointer.As<int32_t>(), clamp );
					break;

				case ScalarTypes::Signed64:
					RangeCastInteger( bson_iter_int64( iterator ), pointer.As<int64_t>(), clamp );
					break;

				case ScalarTypes::Float32:
					RangeCastFloat( bson_iter_int64( iterator ), pointer.As<float32_t>(), clamp );
					break;

				case ScalarTypes::Float64:
					RangeCastFloat( bson_iter_int64( iterator ), pointer.As<float64_t>(), clamp );
					break;
				}
			}
			break;
		}

	case BSON_TYPE_DOUBLE:
		{
			if ( translator->IsA( MetaIds::ScalarTranslator ) )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				bool clamp = true;
				switch ( scalar->m_Type )
				{
				case ScalarTypes::Boolean:
				case ScalarTypes::String:
					break;
				
				case ScalarTypes::Unsigned8:
					RangeCastInteger( bson_iter_double( iterator ), pointer.As<uint8_t>(), clamp );
					break;

				case ScalarTypes::Unsigned16:
					RangeCastInteger( bson_iter_double( iterator ), pointer.As<uint16_t>(), clamp );
					break;

				case ScalarTypes::Unsigned32:
					RangeCastInteger( bson_iter_double( iterator ), pointer.As<uint32_t>(), clamp );
					break;

				case ScalarTypes::Unsigned64:
					RangeCastInteger( bson_iter_double( iterator ), pointer.As<uint64_t>(), clamp );
					break;

				case ScalarTypes::Signed8:
					RangeCastInteger( bson_iter_double( iterator ), pointer.As<int8_t>(), clamp );
					break;

				case ScalarTypes::Signed16:
					RangeCastInteger( bson_iter_double( iterator ), pointer.As<int16_t>(), clamp );
					break;

				case ScalarTypes::Signed32:
					RangeCastInteger( bson_iter_double( iterator ), pointer.As<int32_t>(), clamp );
					break;

				case ScalarTypes::Signed64:
					RangeCastInteger( bson_iter_double( iterator ), pointer.As<int64_t>(), clamp );
					break;

				case ScalarTypes::Float32:
					RangeCastFloat( bson_iter_double( iterator ), pointer.As<float32_t>(), clamp );
					break;

				case ScalarTypes::Float64:
					RangeCastFloat( bson_iter_double( iterator ), pointer.As<float64_t>(), clamp );
					break;
				}
			}
			break;
		}

	case BSON_TYPE_UTF8:
		{
			if ( translator->IsA( MetaIds::ScalarTranslator ) )
			{
				ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
				if ( scalar->m_Type == ScalarTypes::String )
				{
					uint32_t length = 0;
					String str ( bson_iter_utf8( iterator, &length ), length );
					scalar->Parse( str, pointer, this, m_Flags | ArchiveFlags::Notify ? true : false );
				}
			}
			break;
		}

	case BSON_TYPE_ARRAY:
		{
			if ( translator->GetMetaId() == MetaIds::SetTranslator )
			{
				SetTranslator* set = static_cast< SetTranslator* >( translator );
				Translator* itemTranslator = set->GetItemTranslator();

				bson_iter_t child;
				bson_iter_recurse( iterator, &child );

				while( bson_iter_next( &child ) )
				{
					Variable item ( itemTranslator );
					DeserializeTranslator( &child, item, itemTranslator, field, object );
					set->InsertItem( pointer, item );
				}
			}
			else if ( translator->GetMetaId() == MetaIds::SequenceTranslator )
			{
				SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );
				Translator* itemTranslator = sequence->GetItemTranslator();

				bson_iter_t child;
				bson_iter_recurse( iterator, &child );

				while( bson_iter_next( &child ) )
				{
					Variable item ( itemTranslator );
					DeserializeTranslator( &child, item, itemTranslator, field, object );
					sequence->Insert( pointer, sequence->GetLength( pointer ), item );
				}
			}
			break;
		}

	case BSON_TYPE_DOCUMENT:
		{
			if ( translator->GetMetaId() == MetaIds::StructureTranslator )
			{
				StructureTranslator* structure = static_cast< StructureTranslator* >( translator );

				bson_iter_t child;
				bson_iter_recurse( iterator, &child );
				DeserializeInstance( &child, pointer.m_Address,  structure->GetMetaStruct(), object );
			}
			else if ( translator->GetMetaId() == MetaIds::AssociationTranslator )
			{
				AssociationTranslator* assocation = static_cast< AssociationTranslator* >( translator );
				ScalarTranslator* keyTranslator = assocation->GetKeyTranslator();
				Translator* valueTranslator = assocation->GetValueTranslator();

				bson_iter_t child;
				bson_iter_recurse( iterator, &child );

				while( bson_iter_next( &child ) )
				{
					Variable keyVariable ( keyTranslator );
					Variable valueVariable ( valueTranslator );
					String key ( bson_iter_key( &child ) );
					keyTranslator->Parse( key, keyVariable, m_Resolver );
					DeserializeTranslator( &child, valueVariable, valueTranslator, field, object );
					assocation->SetItem( pointer, keyVariable, valueVariable );
				}
			}
			break;
		}

	case BSON_TYPE_DATE_TIME:
		{
			if ( translator->GetMetaId() == MetaIds::StructureTranslator )
			{
				StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
				const MetaStruct* metaStruct = structure->GetMetaStruct();
				if ( metaStruct == Reflect::GetMetaStruct< BsonDateTime >() )
				{
					pointer.As< BsonDateTime >().millis = bson_iter_date_time( iterator );
				}
			}
			break;
		}

	case BSON_TYPE_OID:
		{
			if ( translator->GetMetaId() == MetaIds::StructureTranslator )
			{
				StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
				const MetaStruct* metaStruct = structure->GetMetaStruct();
				if ( metaStruct == Reflect::GetMetaStruct< BsonObjectId >() )
				{
					MemoryCopy( pointer.As< BsonObjectId >().bytes, bson_iter_oid( iterator ), sizeof( bson_oid_t ) );
				}
			}
			break;
		}

	default:
		break;
	}
}
