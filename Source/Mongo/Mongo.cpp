#include "Precompile.h"
#include "Mongo/Mongo.h"

#include "Foundation/Log.h"
#include "Foundation/MemoryStream.h"

HELIUM_DEFINE_CLASS( Helium::Mongo::Model );

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;
using namespace Helium::Mongo;

static String GetNamespace( const String& dbName, const char* collection, const MetaClass* metaClass )
{
	String ns ( dbName );
	ns += ".";

	if ( collection )
	{
		ns += collection;
		return ns;
	}

	std::string defaultCollection;
	if ( metaClass->GetProperty( "defaultCollection", defaultCollection ) )
	{
		ns += defaultCollection.c_str();
		return ns;
	}

	String metaClassName ( metaClass->m_Name );
	for ( size_t startIndex = 0, foundIndex = Invalid<size_t>(); ( foundIndex = metaClassName.Find( ':', startIndex ) ) != Invalid<size_t>(); )
	{
		metaClassName.Remove( foundIndex, 1 );
	}

	ns += metaClassName;
	return ns;
}

void Model::PopulateMetaType( Helium::Reflect::MetaStruct& type )
{
	// some db interactions require _id NOT be set (update), so discard the value but load it if we find it in data
	type.AddField( &Model::id, "_id", Reflect::FieldFlags::Discard );
}

namespace Helium
{
	namespace Mongo
	{
		int initCount = 0;
		int number = 0;

		static int Increment()
		{
			int result = Helium::AtomicIncrement( number );
			return result;
		}
	}
}

void Mongo::Initialize()
{
	if ( ++Mongo::initCount == 1 )
	{
		mongo_init_sockets();
		bson_set_oid_inc( &Increment );

		ArchiveWriterBson::Startup();
		ArchiveReaderBson::Startup();
	}
}

void Mongo::Cleanup()
{
	if ( --Mongo::initCount == 0 )
	{
		bson_set_oid_inc( NULL );

		ArchiveWriterBson::Shutdown();
		ArchiveReaderBson::Shutdown();
	}
}

const char* Mongo::GetErrorString( int status )
{
	switch( status )
	{
	case MONGO_CONN_NO_SOCKET:          return "Could not create a socket";
	case MONGO_CONN_FAIL:               return "An error occured while calling connect()";
	case MONGO_CONN_ADDR_FAIL:          return "An error occured while calling getaddrinfo()";
	case MONGO_CONN_NOT_MASTER:         return "Warning: connected to a non-master node (read-only)";
	case MONGO_CONN_BAD_SET_NAME:       return "Given rs name doesn't match this replica set";
	case MONGO_CONN_NO_PRIMARY:         return "Can't find primary in replica set. Connection closed";
	case MONGO_IO_ERROR:                return "An error occurred while reading or writing on the socket";
	case MONGO_SOCKET_ERROR:            return "Other socket error";
	case MONGO_READ_SIZE_ERROR:         return "The response is not the expected length";
	case MONGO_COMMAND_FAILED:          return "The command returned with 'ok' value of 0";
	case MONGO_WRITE_ERROR:             return "Write with given write_concern returned an error";
	case MONGO_NS_INVALID:              return "The name for the ns (database or collection) is invalid";
	case MONGO_BSON_INVALID:            return "BSON not valid for the specified op";
	case MONGO_BSON_NOT_FINISHED:       return "BSON object has not been finished";
	case MONGO_BSON_TOO_LARGE:          return "BSON object exceeds max BSON size";
	case MONGO_WRITE_CONCERN_INVALID:   return "Supplied write concern object is invalid";
	}

	return "Unknown error";
}

Cursor::Cursor( Database* db, mongo_cursor* cursor )
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
		mongo_cursor_destroy( this->cursor );
	}
}

Helium::StrongPtr< Model > Cursor::Next( const Reflect::MetaClass* defaultType )
{
	if ( !HELIUM_VERIFY( db ) || !HELIUM_VERIFY( cursor ) || !HELIUM_VERIFY_MSG( db->IsCorrectThread(), "Database access from improper thread" ) )
	{
		return NULL;
	}

	Helium::StrongPtr< Model > object;

	while ( !object.ReferencesObject() && mongo_cursor_next( cursor ) == MONGO_OK )
	{
		const Reflect::MetaClass* type = defaultType;

		// if we have type info encoded in the BSON, use it instead of the defaultType
		bson_iterator i[1];
		if ( BSON_STRING == bson_find( i, &cursor->current, "_type" ) )
		{
			const char* typeName = bson_iterator_string( i );
			HELIUM_ASSERT( typeName );
			const Reflect::MetaClass* storedType = Reflect::Registry::GetInstance()->GetMetaClass( typeName );
			if ( storedType )
			{
				type = storedType;
			}
		}

		if ( type )
		{
			object = Reflect::AssertCast< Model >( type->m_Creator() );
		}

		if ( object.ReferencesObject() )
		{
			bson_iterator_init( i, &cursor->current );

			try
			{
				Helium::Persist::ArchiveReaderBson::ReadFromBson( i, reinterpret_cast< Helium::Reflect::ObjectPtr& >( object ) );
			}
			catch ( Helium::Exception& ex )
			{
				Helium::Log::Error( "Failed to parse BSON in query result: %s\n", ex.What() );
				object = NULL;
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

	if ( mongo_cursor_next( cursor ) == MONGO_OK )
	{
		bson_iterator i[1];
		bson_iterator_init( i, &cursor->current );
		try
		{
			Helium::Persist::ArchiveReaderBson::ReadFromBson( i, reinterpret_cast< const Helium::Reflect::ObjectPtr& >( object ) );
		}
		catch ( Helium::Exception& ex )
		{
			Helium::Log::Error( "Failed to generate BSON for object: %s\n", ex.What() );
			result = false;
		}
	}

	return result;
}

Database::Database( const char* name )
	: name( name )
	, isConnected( false )
	, threadId( Thread::GetCurrentId() )
{
	mongo_init( conn );
}

Database::~Database()
{
	mongo_destroy( conn );
}

bool Database::Connect( const char* addr, uint16_t port )
{
	this->threadId = Thread::GetCurrentId();

	if ( MONGO_OK == mongo_client( conn, addr, port ) )
	{
		return isConnected = true;
	}
	else
	{
		Log::Error( "mongo_client failed: %s\n", GetErrorString( conn->err ) );
		return isConnected = false;
	}
}

int64_t Database::GetServerTime( bool inMilliseconds )
{
	bson b[1];
	HELIUM_VERIFY( BSON_OK == bson_init( b ) );
	HELIUM_VERIFY( BSON_OK == bson_append_int( b, "hostInfo", 1 ) );
	HELIUM_VERIFY( BSON_OK == bson_finish( b ) );

	bson_date_t result = -1;

	bson out[1];
	bson_init_zero( out );
	if ( MONGO_OK == mongo_run_command( conn, "admin", b, out ) )
	{
		bson_iterator i[1];
		if ( BSON_OBJECT == bson_find( i, out, "system" ) )
		{
			bson systemBson[1];
			bson_init_zero( systemBson );
			bson_iterator_subobject_init( i, systemBson, false );
			if ( BSON_DATE == bson_find( i, systemBson, "currentTime" ) )
			{
				result = inMilliseconds ? bson_iterator_date( i ) : bson_iterator_date( i ) / 1000;
			}
			bson_destroy( systemBson );
		}
	}

	bson_destroy( b );
	bson_destroy( out );

	return result;
}

bool Database::Drop()
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	if ( MONGO_OK != mongo_cmd_drop_db( conn, name.GetData() ) )
	{
		Log::Error( "mongo_cmd_drop_db failed: %s\n", GetErrorString( conn->err ) );
		return false;
	}

	return true;
}

bool Database::DropCollection( const char* name )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	if ( MONGO_OK != mongo_cmd_drop_collection( conn, this->name.GetData(), name, NULL ) )
	{
		Log::Error( "mongo_cmd_drop_collection failed: %s\n", GetErrorString( conn->err ) );
		return false;
	}

	return true;
}

double Database::GetCollectionCount( const char* name )
{
	return mongo_count( conn, this->name.GetData(), name, NULL );
}

bool Database::CreateCappedCollection( const char* name, int cappedSizeInBytes, int cappedMaxCount )
{
	if ( !HELIUM_VERIFY( IsCorrectThread() ) )
	{
		return false;
	}

	if ( HELIUM_VERIFY( cappedSizeInBytes ) )
	{
		int result = mongo_create_capped_collection( conn, this->name.GetData(), name, cappedSizeInBytes, cappedMaxCount, NULL );
		if ( MONGO_OK != result && MONGO_COMMAND_FAILED != conn->err )
		{
			Log::Error( "mongo_create_capped_collection failed: %s\n", GetErrorString( conn->err ) );
			// call IsConnected to update the value of the isConnected flag.
			IsConnected( true );
			return false;
		}
	}

	return true;
}

bool Database::Insert( const StrongPtr< Model >& object, const char* collection )
{
	if ( !HELIUM_VERIFY( object->id == BsonObjectId::Null ) || !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	bool result = true;

	bson_oid_t oid[1];
	bson_oid_gen( oid );
	MemoryCopy( object->id.bytes, oid->bytes, sizeof( bson_oid_t ) );

	bson b[1];
	HELIUM_VERIFY( BSON_OK == bson_init( b ) );
	try
	{
		bson_oid_t oid[1];
		MemoryCopy( oid, object->id.bytes, sizeof( bson_oid_t ) );	
		HELIUM_VERIFY( BSON_OK == bson_append_oid( b, "_id", oid ) );
		HELIUM_VERIFY( BSON_OK == bson_append_string( b, "_type", object->GetMetaClass()->m_Name ) );
		ArchiveWriterBson::WriteToBson( object, b );
	}
	catch ( Helium::Exception& )
	{
		result = false;
	}
	HELIUM_VERIFY( BSON_OK == bson_finish( b ) );

	if ( result )
	{
		String ns = GetNamespace( name, collection, object->GetMetaClass() );
		if ( MONGO_OK != mongo_insert( conn, ns.GetData(), b, NULL ) )
		{
			Log::Error( "mongo_insert failed: %s\n", GetErrorString( conn->err ) );
			// call IsConnected to update the value of the isConnected flag.
			IsConnected( true );
			result = false;
		}
	}

	bson_destroy( b );
	return result;
}

bool Database::Update( const StrongPtr< Model >& object, const char* collection )
{
	if ( !HELIUM_VERIFY_MSG( object->id != BsonObjectId::Null, "Cannot update object with null id" ) || !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	bson_oid_t oid[1];
	MemoryCopy( oid, object->id.bytes, sizeof( bson_oid_t ) );
#if HELIUM_DEBUG
	char str[ sizeof(bson_oid_t) * 2 + 1 ] = { 0 };
	bson_oid_to_string( oid, str );
#endif

	bson cond[1];
	HELIUM_VERIFY( BSON_OK == bson_init( cond ) );
	HELIUM_VERIFY( BSON_OK == bson_append_oid( cond, "_id", oid ) );
	HELIUM_VERIFY( BSON_OK == bson_finish( cond ) );

	bson op[1];
	HELIUM_VERIFY( BSON_OK == bson_init( op ) );
	bool result = true;
	try
	{
		HELIUM_VERIFY( BSON_OK == bson_append_start_object( op, "$set" ) );
		ArchiveWriterBson::WriteToBson( object, op );
		HELIUM_VERIFY( BSON_OK == bson_append_finish_object( op ) );
	}
	catch ( Helium::Exception& )
	{
		Log::Error( "Failed to generate BSON for object\n" );
		result = false;
	}
	HELIUM_VERIFY( BSON_OK == bson_finish( op ) );

	if ( result )
	{
		String ns = GetNamespace( name, collection, object->GetMetaClass() );
		if ( MONGO_OK != mongo_update( conn, ns.GetData(), cond, op, MONGO_UPDATE_BASIC, NULL ) )
		{
			Log::Error( "mongo_update failed: %s\n", GetErrorString( conn->err ) );
			// call IsConnected to update the value of the isConnected flag.
			IsConnected( true );
			result = false;
		}
	}

	bson_destroy( cond );
	bson_destroy( op );
	return result;
}

bool Database::Get( const StrongPtr< Model >& object, const char* collection )
{
	if ( !HELIUM_VERIFY_MSG( object->id != BsonObjectId::Null, "Cannot update object with null id" ) || !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	bool result = true;

	bson_oid_t oid[1];
	MemoryCopy( oid, object->id.bytes, sizeof( bson_oid_t ) );
#if HELIUM_DEBUG
	char str[ sizeof(bson_oid_t) * 2 + 1 ] = { 0 };
	bson_oid_to_string( oid, str );
#endif

	bson query[1];
	HELIUM_VERIFY( BSON_OK == bson_init( query ) );
	HELIUM_VERIFY( BSON_OK == bson_append_oid( query, "_id", oid ) );
	HELIUM_VERIFY( BSON_OK == bson_finish( query ) );

	bson out[1];
	bson_init_zero( out );

	{
		String ns = GetNamespace( name, collection, object->GetMetaClass() );
		if ( MONGO_OK != mongo_find_one( conn, ns.GetData(), query, NULL, out ) )
		{
			Log::Error( "mongo_find_one failed: %s\n", GetErrorString( conn->err ) );
			// call IsConnected to update the value of the isConnected flag.
			IsConnected( true );
			result = false;
		}
	}

	if ( result )
	{
		bson_iterator i[1];
		bson_iterator_init( i, out );
		try
		{
			ArchiveReaderBson::ReadFromBson( i, reinterpret_cast< Helium::StrongPtr< Object >& >( const_cast< StrongPtr< Model >& >( object ) ) );
		}
		catch ( Helium::Exception& )
		{
			Log::Error( "Failed to generate object for BSON\n" );
			result = false;
		}
	}

	bson_destroy( query );
	bson_destroy( out );
	return result;
}

bool Database::Insert( StrongPtr< Model >* objects, size_t count, const char* collection )
{
	if ( !HELIUM_VERIFY( objects ) || !HELIUM_VERIFY( count ) || !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	bool result = true;
	const MetaClass* metaClass = (*objects)->GetMetaClass();
	Helium::DynamicArray< bson > bsons;
	Helium::DynamicArray< bson* > pointers;
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

		bsons.Add( bson() );
		bson* b = &bsons.GetLast();
		HELIUM_VERIFY( BSON_OK == bson_init( b ) );
		pointers.Add( b );

		bson_oid_gen( (bson_oid_t*)objects[i]->id.bytes );

#if HELIUM_DEBUG
		char str[ sizeof(bson_oid_t) * 2 + 1 ] = { 0 };
		bson_oid_to_string( (bson_oid_t*)objects[i]->id.bytes, str );
#endif

		try
		{
			bson_oid_t oid[1];
			MemoryCopy( oid, objects[i]->id.bytes, sizeof( bson_oid_t ) );
			HELIUM_VERIFY( BSON_OK == bson_append_oid( b, "_id", oid ) );
			HELIUM_VERIFY( BSON_OK == bson_append_string( b, "_type", objects[i]->GetMetaClass()->m_Name ) );
			ArchiveWriterBson::WriteToBson( reinterpret_cast< ObjectPtr& >( objects[i] ), b );
		}
		catch ( Helium::Exception& )
		{
			Log::Error( "Failed to generate BSON for object\n" );
			result = false;
		}

		HELIUM_VERIFY( BSON_OK == bson_finish( b ) );
	}

	if ( result )
	{
		String ns = GetNamespace( name, collection, metaClass );
		if ( MONGO_OK != mongo_insert_batch( conn, ns.GetData(), const_cast< const bson** >( pointers.GetData() ), static_cast< int >( pointers.GetSize() ), NULL, 0x0 ) )
		{
			Log::Error( "mongo_insert_batch failed: %s\n", GetErrorString( conn->err ) );
			// call IsConnected to update the value of the isConnected flag.
			IsConnected( true );
			result = false;
		}
	}

	for ( size_t i=0; i<pointers.GetSize(); ++i )
	{
		bson_destroy( pointers[i] );
	}

	return result;
}

bool Database::EnsureIndex( const char* collection, const bson* key, const char* name, int options )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	Helium::String ns ( this->name );
	ns += ".";
	ns += collection;

	if ( MONGO_OK != mongo_create_index( conn, ns.GetData(), key, name, options, NULL ) )
	{
		Log::Error( "mongo_cmd_drop_collection failed: %s\n", GetErrorString( conn->err ) );
		return false;
	}

	return true;
}

Cursor Database::Find( const char* collection, const bson* query, int limit, int skip, int options )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return Cursor();
	}

	Helium::String ns ( name );
	ns += ".";
	ns += collection;

	// we don't currently support specifying fields because we need to ensure _type is sent back, and
	//  merging bson documents is difficult/impossible with the current version of the bson api
	mongo_cursor* c = mongo_find( conn, ns.GetData(), query, NULL, limit, skip, options );
	if ( c )
	{
		return Cursor( this, c );
	}
	else
	{
		return Cursor();
	}
}


bool Database::Remove( const char* collection, const bson* query )
{
	if ( !HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		return false;
	}

	Helium::String ns ( name );
	ns += ".";
	ns += collection;

	if ( MONGO_OK != mongo_remove( conn, ns.GetData(), query, NULL ) )
	{
		Log::Error( "mongo_cmd_remove failed: %s\n", GetErrorString( conn->err ) );
		return false;
	}

	return true;
}
