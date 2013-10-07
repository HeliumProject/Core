template< class T >
Helium::Mongo::Cursor< T >::Cursor( int options )
: db( NULL )
, cursor( NULL )
, options( options )
{
}

template< class T >
Helium::Mongo::Cursor< T >::~Cursor()
{
	Set( NULL, NULL );
}

template< class T >
void Helium::Mongo::Cursor< T >::Set( Database* db, mongo_cursor* cursor )
{
	this->db = db;
	mongo_cursor_destroy( this->cursor );
	this->cursor = cursor;
}

template< class T >
bool Helium::Mongo::Cursor< T >::Get( Helium::DynamicArray< Helium::StrongPtr< T > >& objects, size_t count )
{
	if ( !HELIUM_VERIFY( db ) || !HELIUM_VERIFY( cursor ) || !HELIUM_VERIFY_MSG( db->threadId == Helium::GetCurrentThreadID(), "Database access from improper thread" ) )
	{
		return false;
	}

	bool result = true;

	while( mongo_cursor_next( cursor ) == MONGO_OK && count-- != 0 )
	{
		bson_iterator i[1];
		bson_iterator_init( i, &cursor->current );
		Helium::StrongPtr< T > o = new T;
		try
		{
			Helium::Persist::ArchiveReaderBson::ReadFromBson( i, reinterpret_cast< Helium::Reflect::ObjectPtr& >( o ) );
			objects.Add( o );
		}
		catch ( Helium::Exception& ex )
		{
			Helium::Log::Error( "Failed to generate BSON for object: %s\n", ex.What() );
		}
	}

	return result;
}

template< class T >
Helium::StrongPtr< T > Helium::Mongo::Cursor< T >::Next()
{
	if ( !HELIUM_VERIFY( db ) || !HELIUM_VERIFY( cursor ) || !HELIUM_VERIFY_MSG( db->threadId == Helium::GetCurrentThreadID(), "Database access from improper thread" ) )
	{
		return NULL;
	}

	if ( mongo_cursor_next( cursor ) == MONGO_OK )
	{
		bson_iterator i[1];
		bson_iterator_init( i, &cursor->current );
		Helium::StrongPtr< T > o = new T;
		try
		{
			Helium::Persist::ArchiveReaderBson::ReadFromBson( i, reinterpret_cast< Helium::Reflect::ObjectPtr& >( o ) );
			return o;
		}
		catch ( Helium::Exception& ex )
		{
			Helium::Log::Error( "Failed to generate BSON for object: %s\n", ex.What() );
		}
	}

	return NULL;
}

#if HELIUM_CPP11
template< class T >
void Helium::Mongo::Cursor< T >::Process( std::function< void ( T* ) > function )
{
	if ( !HELIUM_VERIFY( db ) || !HELIUM_VERIFY( cursor ) || !HELIUM_VERIFY_MSG( db->threadId == Helium::GetCurrentThreadID(), "Database access from improper thread" ) )
	{
		return;
	}

	bool isOk = true;
	while( isOk )
	{
		isOk = mongo_cursor_next( cursor ) == MONGO_OK;
		if ( !isOk )
		{
			break;
		}

		bson_iterator i[1];
		bson_iterator_init( i, &cursor->current );
		Helium::StrongPtr< T > o = new T;
		try
		{
			Helium::Persist::ArchiveReaderBson::ReadFromBson( i, reinterpret_cast< Helium::Reflect::ObjectPtr& >( o ) );
			function( o );
		}
		catch ( Helium::Exception& ex )
		{
			Helium::Log::Error( "Failed to generate BSON for object: %s\n", ex.What() );
		}
	}
}
#endif

template< class T >
int Helium::Mongo::Cursor< T >::GetOptions()
{
	return this->options;
}

template< class T >
void Helium::Mongo::Cursor< T >::SetOptions( int options )
{
	this->options = options;
}

bool Helium::Mongo::Database::IsConnected() const
{
	return isConnected;
}

void Helium::Mongo::Database::SetThread( Helium::Thread::id_t threadId )
{
	this->threadId = threadId;
}

template< class T >
bool Helium::Mongo::Database::Insert( const Helium::StrongPtr< T >& object )
{
	return Insert( static_cast< const Helium::StrongPtr< Model >& >( object ), T::defaultCollection );
}

template< class T >
bool Helium::Mongo::Database::Update( const Helium::StrongPtr< T >& object )
{
	return Update( static_cast< const Helium::StrongPtr< Model >& >( object ), T::defaultCollection );
}

template< class T >
bool Helium::Mongo::Database::Get( const Helium::StrongPtr< T >& object )
{
	return Get( static_cast< const Helium::StrongPtr< Model >& >( object ), T::defaultCollection );
}

template< class T >
bool Helium::Mongo::Database::Insert( Helium::StrongPtr< T >* objects, size_t count )
{
	return Insert( static_cast< Helium::StrongPtr< Model >* >( objects ), count, T::defaultCollection );
}

template< class T >
bool Helium::Mongo::Database::Find( Cursor< T >& result, const bson* query, const bson* fields, const char* collection, int limit, int skip )
{
	if ( !HELIUM_VERIFY_MSG( this->threadId == Helium::GetCurrentThreadID(), "Database access from improper thread" ) )
	{
		return false;
	}

	Helium::String ns ( name );
	ns += ".";
	ns += collection ? collection : ( T::defaultCollection ? T::defaultCollection : Helium::Reflect::GetMetaClass< T >()->m_Name );

	mongo_cursor* cursor = mongo_find( conn, ns.GetData(), query, fields, limit, skip, result.GetOptions() );
	if ( cursor )
	{
		result.Set( this, cursor );
	}

	return cursor != NULL;
}

