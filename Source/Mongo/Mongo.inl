bool Helium::Mongo::Cursor::IsValid()
{
	return db != NULL && cursor != NULL;
}

template< class DefaultType >
Helium::StrongPtr< DefaultType > Helium::Mongo::Cursor::Next()
{
	return Reflect::SafeCast< DefaultType >( Next( Reflect::GetMetaClass< DefaultType >() ) );
}

#if !HELIUM_SHARED
mongoc_client_t* Helium::Mongo::Database::GetClient()
{
	return client;
}

mongoc_database_t* Helium::Mongo::Database::GetDatabase()
{
	return database;
}
#endif

void Helium::Mongo::Database::SetThread( Helium::ThreadId threadId )
{
	if ( this->threadId != threadId )
	{
		this->threadId = threadId;
	}
}

bool Helium::Mongo::Database::IsCorrectThread() const
{
	return this->threadId == Thread::GetCurrentId();
}