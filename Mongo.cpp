#include "Mongo/MongoPch.h"
#include "Mongo/Mongo.h"

#include "Foundation/Log.h"
#include "Foundation/MemoryStream.h"

HELIUM_DEFINE_CLASS( Helium::Mongo::Model );

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;
using namespace Helium::Mongo;

const char* Model::defaultCollection = NULL;

void Model::PopulateMetaType( Helium::Reflect::MetaStruct& type )
{
	type.AddField( &Model::id, "_id" );
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
	}
}

void Mongo::Cleanup()
{
	if ( --Mongo::initCount == 0 )
	{
		bson_set_oid_inc( NULL );
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

Database::Database( const char* name )
	: name( name )
	, isConnected( false )
	, threadId( Helium::GetCurrentThreadID() )
{
	mongo_init( conn );
}

Database::~Database()
{
	mongo_destroy( conn );
}

void Database::SetName( const char* name )
{
	this->name = name;
}

void Database::SetTimeout( int timeoutMilliseconds )
{
	mongo_set_op_timeout( conn, timeoutMilliseconds );
}

bool Database::Connect( const char* addr, uint16_t port )
{
	this->threadId = Helium::GetCurrentThreadID();

	if ( MONGO_OK == mongo_client( conn, addr, port ) )
	{
		return isConnected = true;
	}
	else
	{
		Log::Error( "Mongo: %s\n", GetErrorString( conn->err ) );
		return isConnected = false;
	}
}

double Database::GetCollectionCount( const char* name )
{
	return mongo_count( conn, this->name.GetData(), name, NULL );
}

bool Database::CreateCappedCollection( const char* name, int cappedSizeInBytes, int cappedMaxCount )
{
	if ( !HELIUM_VERIFY( this->threadId == Helium::GetCurrentThreadID() ) )
	{
		return false;
	}

	if ( HELIUM_VERIFY( cappedSizeInBytes ) )
	{
		int result = mongo_create_capped_collection( conn, this->name.GetData(), name, cappedSizeInBytes, cappedMaxCount, NULL );
		if ( MONGO_OK != result && MONGO_COMMAND_FAILED != conn->err )
		{
			Log::Error( "Mongo: %s\n", GetErrorString( conn->err ) );
			return false;
		}
	}

	return true;
}

bool Database::Insert( const StrongPtr< Model >& object, const char* collection )
{
	if ( !HELIUM_VERIFY( object->id == BsonObjectId::Null ) || !HELIUM_VERIFY_MSG( this->threadId == Helium::GetCurrentThreadID(), "Database access from improper thread" ) )
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
		ArchiveWriterBson::ToBson( object, b );
	}
	catch ( Helium::Exception& )
	{
		result = false;
	}
	HELIUM_VERIFY( BSON_OK == bson_finish( b ) );

	if ( result )
	{
		const MetaClass* metaClass = object->GetMetaClass();

		String ns ( name );
		ns += ".";
		ns += collection ? collection : metaClass->m_Name;

		if ( MONGO_OK != mongo_insert( conn, ns.GetData(), b, NULL ) )
		{
			Log::Error( "Mongo: %s\n", GetErrorString( conn->err ) );
			result = false;
		}
	}

	bson_destroy( b );
	return result;
}

bool Database::Update( const StrongPtr< Model >& object, const char* collection )
{
	if ( !HELIUM_VERIFY_MSG( object->id != BsonObjectId::Null, "Cannot update object with null id" ) || !HELIUM_VERIFY_MSG( this->threadId == Helium::GetCurrentThreadID(), "Database access from improper thread" ) )
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
		ArchiveWriterBson::ToBson( object, op, "$set" );
	}
	catch ( Helium::Exception& )
	{
		Log::Error( "Failed to generate BSON for object\n" );
		result = false;
	}
	HELIUM_VERIFY( BSON_OK == bson_finish( op ) );

	if ( result )
	{
		const MetaClass* metaClass = object->GetMetaClass();

		String ns ( name );
		ns += ".";
		ns += collection ? collection : metaClass->m_Name;

		if ( MONGO_OK != mongo_update( conn, ns.GetData(), cond, op, MONGO_UPDATE_BASIC, NULL ) )
		{
			Log::Error( "Mongo: %s\n", GetErrorString( conn->err ) );
			result = false;
		}
	}

	bson_destroy( cond );
	bson_destroy( op );
	return result;
}

bool Database::Get( const StrongPtr< Model >& object, const char* collection )
{
	if ( !HELIUM_VERIFY_MSG( object->id != BsonObjectId::Null, "Cannot update object with null id" ) || !HELIUM_VERIFY_MSG( this->threadId == Helium::GetCurrentThreadID(), "Database access from improper thread" ) )
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
		const MetaClass* metaClass = object->GetMetaClass();

		String ns ( name );
		ns += ".";
		ns += collection ? collection : metaClass->m_Name;

		if ( MONGO_OK != mongo_find_one( conn, ns.GetData(), query, NULL, out ) )
		{
			Log::Error( "Mongo: %s\n", GetErrorString( conn->err ) );
			result = false;
		}
	}

	if ( result )
	{
		bson_iterator i[1];
		bson_iterator_init( i, out );
		try
		{
			ArchiveReaderBson::FromBson( i, reinterpret_cast< Helium::StrongPtr< Object >& >( const_cast< StrongPtr< Model >& >( object ) ) );
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
	if ( !HELIUM_VERIFY( objects ) || !HELIUM_VERIFY( count ) || !HELIUM_VERIFY_MSG( this->threadId == Helium::GetCurrentThreadID(), "Database access from improper thread" ) )
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
			ArchiveWriterBson::ToBson( reinterpret_cast< ObjectPtr& >( objects[i] ), b );
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
		String ns ( name );
		ns += ".";
		ns += collection ? collection : metaClass->m_Name;

		if ( MONGO_OK != mongo_insert_batch( conn, ns.GetData(), const_cast< const bson** >( pointers.GetData() ), static_cast< int >( pointers.GetSize() ), NULL, 0x0 ) )
		{
			Log::Error( "Mongo: %s\n", GetErrorString( conn->err ) );
			result = false;
		}
	}

	for ( size_t i=0; i<pointers.GetSize(); ++i )
	{
		bson_destroy( pointers[i] );
	}

	return result;
}
