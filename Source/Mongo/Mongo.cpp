#include "Precompile.h"
#include "Mongo/Mongo.h"

#include "Foundation/Log.h"
#include "Foundation/MemoryStream.h"

HELIUM_DEFINE_CLASS( Helium::Mongo::Model );

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;
using namespace Helium::Mongo;

static String FinalizeCollectionName( const char* collectionName, const MetaClass* metaClass )
{
	if ( collectionName )
	{
		return String(collectionName);
	}

	std::string defaultCollection;
	if ( metaClass->GetProperty( "defaultCollection", defaultCollection ) )
	{
		return String(defaultCollection.c_str());
	}

	String metaClassName ( metaClass->m_Name );
	for ( size_t startIndex = 0, foundIndex = Invalid<size_t>(); ( foundIndex = metaClassName.Find( ':', startIndex ) ) != Invalid<size_t>(); )
	{
		metaClassName.Remove( foundIndex, 1 );
	}

	return metaClassName;
}

void Model::PopulateMetaType( Helium::Reflect::MetaStruct& type )
{
	// some db interactions require _id NOT be set (update), so discard the value but load it if we find it in data
	type.AddField( &Model::id, "_id", Reflect::FieldFlags::Discard );
}

static int initCount = 0;

void Mongo::Startup()
{
	if ( ++initCount == 1 )
	{
		mongoc_init();

		ArchiveWriterBson::Startup();
		ArchiveReaderBson::Startup();
	}
}

void Mongo::Shutdown()
{
	if ( --initCount == 0 )
	{
		ArchiveWriterBson::Shutdown();
		ArchiveReaderBson::Shutdown();

		mongoc_cleanup();
	}
}

Cursor::Cursor( Database* db, mongoc_cursor_t* cursor )
	: db( db )
	, cursor( cursor )
{
}

Cursor::Cursor( const Cursor& rhs )
	: db( rhs.db )
	, cursor( rhs.cursor )
{
	rhs.db = NULL;
	rhs.cursor = NULL;
}

Cursor::~Cursor()
{
	if ( this->cursor )
	{
		mongoc_cursor_destroy( this->cursor );
	}
}

Helium::StrongPtr< Model > Cursor::Next( const Reflect::MetaClass* defaultType )
{
	if ( !HELIUM_VERIFY( db ) || !HELIUM_VERIFY( cursor ) || !HELIUM_VERIFY_MSG( db->IsCorrectThread(), "Database access from improper thread" ) )
	{
		return NULL;
	}

	Helium::StrongPtr< Model > object;

	const bson_t* bson = nullptr;
	while ( !object.ReferencesObject() && mongoc_cursor_next( cursor, &bson ) )
	{
		const Reflect::MetaClass* type = defaultType;

		// if we have type info encoded in the BSON, use it instead of the defaultType
		{
			bson_iter_t typeIterator;
			if ( bson_iter_init_find(&typeIterator, bson, "_type" ) && bson_iter_type( &typeIterator ) == BSON_TYPE_UTF8 )
			{
				uint32_t length = 0;
				const char* typeName = bson_iter_utf8( &typeIterator, &length );
				HELIUM_ASSERT( typeName );
				const Reflect::MetaClass* storedType = Reflect::Registry::GetInstance()->GetMetaClass( typeName );
				if ( storedType )
				{
					type = storedType;
				}
			}
		}

		if ( type )
		{
			object = Reflect::AssertCast< Model >( type->m_Creator() );
		}

		if ( object.ReferencesObject() )
		{
			bson_iter_t objectIterator;
			bson_iter_init( &objectIterator, bson );
			try
			{
				Helium::Persist::ArchiveReaderBson::ReadFromBson( &objectIterator, reinterpret_cast< Helium::Reflect::ObjectPtr& >( object ) );
			}
			catch ( Helium::Exception& ex )
			{
				Helium::Log::Error( "Failed to parse BSON in query result: %s\n", ex.What() );
				object = nullptr;
			}
		}
	}

	return object;
}

bool Cursor::Next( const Helium::StrongPtr< Model >& object )
{
	if ( !HELIUM_VERIFY( db ) || !HELIUM_VERIFY( cursor ) || !HELIUM_VERIFY_MSG( db->IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	bool result = true;

	const bson_t* bson = nullptr;
	if ( mongoc_cursor_next( cursor, &bson ) )
	{
		bson_iter_t objectIterator;
		bson_iter_init( &objectIterator, bson );
		try
		{
			Helium::Persist::ArchiveReaderBson::ReadFromBson( &objectIterator, reinterpret_cast< const Helium::Reflect::ObjectPtr& >( object ) );
		}
		catch ( Helium::Exception& ex )
		{
			Helium::Log::Error( "Failed to generate BSON for object: %s\n", ex.What() );
			result = false;
		}
	}
	else
	{
		bson_error_t error;
		const bson_t* reply = nullptr;
		if ( mongoc_cursor_error_document( cursor, &error, &reply ) )
		{
			Log::Error( "mongoc_cursor_next failed: %d.%d: %s\n", error.domain, error.code, error.message );
		}
	}

	return result;
}

static uint32_t g_InitCount = 0;

Database::Database( const char* name )
	: name( name )
	, threadId( 0 )
	, client( nullptr )
	, database( nullptr )
{
}

Database::~Database()
{
	if ( client )
	{
		mongoc_database_destroy( database );
		mongoc_client_destroy( client );
	}
}

bool Database::Connect( const char* serverUriWithDatabase )
{
	this->threadId = Thread::GetCurrentId();

	client = mongoc_client_new( serverUriWithDatabase );
	if ( !client )
	{
		return false;
	}

	database = mongoc_client_get_default_database( client );
	if ( !database )
	{
		mongoc_client_destroy( client );
		return false;
	}

	return HELIUM_VERIFY( (name = mongoc_database_get_name( database )).IsOccupied() );
}

int64_t Database::GetServerTime()
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	bson_t request;
	bson_init( &request );
	HELIUM_VERIFY( bson_append_int32( &request, "hostInfo", -1, 1 ) );

	int64_t serverTime = -1;

	bson_t reply;
	bson_error_t error;
	if ( mongoc_client_command_simple( client, "admin", &request, nullptr, &reply, &error ) )
	{
		bson_iter_t system;
		if ( bson_iter_init_find( &system, &reply, "system" ) && BSON_TYPE_DOCUMENT == bson_iter_type( &system ) )
		{
			bson_iter_t currentTime;
			if ( bson_iter_recurse( &system, &currentTime ) && BSON_TYPE_DATE_TIME == bson_iter_type( &currentTime ) )
			{
				serverTime = bson_iter_date_time( &currentTime );
			}
		}
	}
	else
	{
		Log::Error( "mongoc_client_command_simple failed: %d.%d: %s\n", error.domain, error.code, error.message );
	}

	bson_destroy( &request );
	bson_destroy( &reply );
	return serverTime;
}

bool Database::Drop()
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	bson_t request;
	bson_init( &request );
	HELIUM_VERIFY( bson_append_int32( &request, "dropDatabase", -1, 1 ) );

	bool result = true;
	bson_t reply;
	bson_error_t error;
	if ( !mongoc_client_command_simple( client, this->name.GetData(), &request, nullptr, &reply, &error ) )
	{
		Log::Error( "mongoc_client_command_simple failed: %d.%d: %s\n", error.domain, error.code, error.message );
		result = false;
	}

	bson_destroy( &request );
	bson_destroy( &reply );
	return result;
}

bool Database::DropCollection( const char* collectionName )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	bson_t request;
	bson_init( &request );
	HELIUM_VERIFY( bson_append_utf8( &request, "drop", -1, collectionName, -1 ) );

	bool result = true;
	bson_t reply;
	bson_error_t error;
	if ( !mongoc_client_command_simple( client, this->name.GetData(), &request, nullptr, &reply, &error ) )
	{
		Log::Error( "mongoc_client_command_simple failed: %d.%d: %s\n", error.domain, error.code, error.message );
		result = false;
	}

	bson_destroy( &request );
	bson_destroy( &reply );
	return result;
}

int64_t Database::GetCollectionCount( const char* collectionName, bool estimated )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	mongoc_collection_t* collection = mongoc_database_get_collection( database, collectionName );

	int64_t result;
	bson_error_t error;
	if (estimated)
	{
		result = mongoc_collection_estimated_document_count( collection, nullptr, nullptr, nullptr, &error );
		if ( result < 0 )
		{
			Log::Error( "mongoc_collection_estimated_document_count failed: %d.%d: %s\n", error.domain, error.code, error.message );
		}
	}
	else
	{
		bson_t filter;
		bson_init( &filter );
		result = mongoc_collection_count_documents( collection, &filter, nullptr, nullptr, nullptr, &error );
		if ( result < 0 )
		{
			Log::Error( "mongoc_collection_count_documents failed: %d.%d: %s\n", error.domain, error.code, error.message );
		}
		bson_destroy( &filter );
	}

	return result;
}

bool Database::CreateCappedCollection( const char* collectionName, int cappedSizeInBytes, int cappedMaxCount )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	bson_t request;
	bson_init( &request );
	HELIUM_VERIFY( bson_append_utf8( &request, "create", -1, collectionName, -1 ) );
	HELIUM_VERIFY( bson_append_bool( &request, "capped", -1, true) );
	HELIUM_VERIFY( bson_append_int32( &request, "size", -1, cappedSizeInBytes) );
	HELIUM_VERIFY( bson_append_int32( &request, "max", -1, cappedMaxCount) );

	bool result = false;
	bson_t reply;
	bson_error_t error;
	if ( mongoc_client_command_simple( client, this->name.GetData(), &request, nullptr, &reply, &error ) )
	{
		Log::Error( "mongoc_collection_insert_one failed: %d.%d: %s\n", error.domain, error.code, error.message );
		result = true;
	}

	bson_destroy( &request );
	bson_destroy( &reply );
	return result;
}

bool Database::Insert( const StrongPtr< Model >& object, const char* collectionName )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	if ( !HELIUM_VERIFY( object->id == BsonObjectId::Null ) )
	{
		return false;
	}

	bool result = true;

	bson_oid_t oid;
	bson_oid_init( &oid, nullptr );
	MemoryCopy( object->id.bytes, &oid.bytes, sizeof( bson_oid_t ) );

	bson_t bson;
	bson_init(&bson);
	try
	{
		HELIUM_VERIFY( bson_append_oid( &bson, "_id", -1, &oid ) );
		HELIUM_VERIFY( bson_append_utf8( &bson, "_type", -1, object->GetMetaClass()->m_Name, -1 ) );
		ArchiveWriterBson::WriteToBson( object, &bson );
	}
	catch ( Helium::Exception& )
	{
		result = false;
	}

	if ( result )
	{
		String finalCollectionName = FinalizeCollectionName(collectionName, object->GetMetaClass());

		mongoc_collection_t* collection = mongoc_database_get_collection( database, finalCollectionName.GetData() );

		bson_error_t error;
		if ( !mongoc_collection_insert_one( collection, &bson, nullptr, nullptr, &error ) )
		{
			Log::Error( "mongoc_collection_insert_one failed: %d.%d: %s\n", error.domain, error.code, error.message );
			result = false;
		}
	}

	bson_destroy( &bson );
	return result;
}

bool Database::Update( const StrongPtr< Model >& object, const char* collectionName )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	if ( !HELIUM_VERIFY_MSG( object->id != BsonObjectId::Null, "Cannot update object with null id" ) )
	{
		return false;
	}

	bson_oid_t oid;
	MemoryCopy( &oid, object->id.bytes, sizeof( bson_oid_t ) );

	bson_t query;
	bson_init( &query );
	HELIUM_VERIFY( bson_append_oid( &query, "_id", -1, &oid ) );

	bson_t update;
	bson_init( &update );
	bool result = true;
	try
	{
		bson_t fields;
		HELIUM_VERIFY( bson_append_document_begin( &update, "$set", -1, &fields ) );
		ArchiveWriterBson::WriteToBson( object, &fields );
		HELIUM_VERIFY( bson_append_document_end( &update, &fields ) );
	}
	catch ( Helium::Exception& )
	{
		Log::Error( "Failed to generate BSON for object\n" );
		result = false;
	}

	if ( result )
	{
		String finalCollectionName = FinalizeCollectionName(collectionName, object->GetMetaClass());

		mongoc_collection_t* collection = mongoc_database_get_collection( database, finalCollectionName.GetData() );

		bson_error_t error;
		if ( !mongoc_collection_update_one( collection, &query, &update, nullptr, nullptr, &error ) )
		{
			Log::Error( "mongoc_collection_update_one failed: %d.%d: %s\n", error.domain, error.code, error.message );
			result = false;
		}
	}

	bson_destroy( &query );
	bson_destroy( &update );
	return result;
}

bool Database::Get( const StrongPtr< Model >& object, const char* collectionName )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	if ( !HELIUM_VERIFY_MSG( object->id != BsonObjectId::Null, "Cannot update object with null id" ) )
	{
		return false;
	}

	bool result = true;

	bson_oid_t oid;
	MemoryCopy( &oid, object->id.bytes, sizeof( bson_oid_t ) );

	bson_t query;
	bson_init( &query );
	HELIUM_VERIFY( bson_append_oid( &query, "_id", -1, &oid ) );

	{
		String finalCollectionName = FinalizeCollectionName(collectionName, object->GetMetaClass());

		mongoc_collection_t* collection = mongoc_database_get_collection( database, finalCollectionName.GetData() );

		Cursor cursor ( this, mongoc_collection_find( collection, MONGOC_QUERY_NONE, 0, 1, 0, &query, nullptr, nullptr ) );

		result = cursor.Next( object );
	}

	bson_destroy( &query );
	return result;
}

bool Database::Insert( StrongPtr< Model >* objects, size_t count, const char* collectionName )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	if ( !HELIUM_VERIFY( objects ) || !HELIUM_VERIFY( count ) )
	{
		return false;
	}

	bool result = true;
	const MetaClass* metaClass = (*objects)->GetMetaClass();
	Helium::DynamicArray< bson_t > bsons;
	Helium::DynamicArray< bson_t* > pointers;
	bsons.Reserve( count );
	pointers.Reserve( count );
	for ( size_t i=0; i<count && result; ++i )
	{
		if ( !HELIUM_VERIFY( metaClass == objects[i]->GetMetaClass() ) )
		{
			continue;
		}

		if ( !HELIUM_VERIFY( objects[i]->id == BsonObjectId::Null ) )
		{
			continue;
		}

		bsons.Add( bson_t() );
		bson_t* b = &bsons.GetLast();
		bson_init( b );
		pointers.Add( b );

		bson_oid_t oid;
		bson_oid_init( &oid, nullptr );

		try
		{
			MemoryCopy( objects[i]->id.bytes, &oid, sizeof( bson_oid_t ) );
			HELIUM_VERIFY( bson_append_oid( b, "_id", -1, &oid ) );
			HELIUM_VERIFY( bson_append_utf8( b, "_type", -1, objects[i]->GetMetaClass()->m_Name, -1 ) );
			ArchiveWriterBson::WriteToBson( reinterpret_cast< ObjectPtr& >( objects[i] ), b );
		}
		catch ( Helium::Exception& )
		{
			Log::Error( "Failed to generate BSON for object\n" );
			result = false;
		}
	}

	if ( result )
	{
		String finalCollectionName = FinalizeCollectionName( collectionName, metaClass );

		mongoc_collection_t* collection = mongoc_database_get_collection( database, finalCollectionName.GetData() );

		bson_error_t error;
		if ( !mongoc_collection_insert_many( collection, const_cast< const bson_t** >( pointers.GetData() ), pointers.GetSize(), nullptr, nullptr, &error ) )
		{
			Log::Error( "mongoc_collection_insert_many failed: %d.%d: %s\n", error.domain, error.code, error.message );
			result = false;
		}
	}

	for ( size_t i=0; i<pointers.GetSize(); ++i )
	{
		bson_destroy( pointers[i] );
	}

	return result;
}

bool Database::EnsureIndex( const char* collectionName, const bson_t* key, const char* name, int options )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	mongoc_collection_t* collection = mongoc_database_get_collection( database, collectionName );

	bson_error_t error;
	if ( !mongoc_collection_ensure_index( collection, key, nullptr, &error ) )
	{
		Log::Error( "mongoc_collection_ensure_index failed: %d.%d: %s\n", error.domain, error.code, error.message );
		return false;
	}

	return true;
}

Cursor Database::Find( const char* collectionName, const bson_t* query, int limit, int skip, int options )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return Cursor();
	}

	mongoc_collection_t* collection = mongoc_database_get_collection( database, collectionName );

	// we don't currently support specifying fields because we need to ensure _type is sent back, and
	//  merging bson documents is difficult/impossible with the current version of the bson api
	mongoc_cursor_t* c = mongoc_collection_find( collection, MONGOC_QUERY_NONE, skip, limit, 0, query, nullptr, nullptr );
	if ( c )
	{
		return Cursor( this, c );
	}
	else
	{
		return Cursor();
	}
}

bool Database::Remove( const char* collectionName, const bson_t* query )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	mongoc_collection_t* collection = mongoc_database_get_collection( database, collectionName );

	bson_error_t error;
	if ( !mongoc_collection_remove( collection, MONGOC_REMOVE_NONE, query, nullptr, &error ) )
	{
		Log::Error( "mongoc_collection_ensure_index failed: %d.%d: %s\n", error.domain, error.code, error.message );
		return false;
	}

	return true;
}
