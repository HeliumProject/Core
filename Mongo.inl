bool Helium::Mongo::Cursor::IsValid()
{
	return db != NULL && cursor != NULL;
}

template< class DefaultType >
Helium::StrongPtr< DefaultType > Helium::Mongo::Cursor::Next()
{
	return Reflect::SafeCast< DefaultType >( Next( Reflect::GetMetaClass< DefaultType >() ) );
}

inline const char* Helium::Mongo::Database::GetName() const
{
	return this->name.GetData();
}

inline void Helium::Mongo::Database::SetName( const char* name )
{
	this->name = name;
}

bool Helium::Mongo::Database::IsConnected( bool pingServer )
{
	if ( isConnected && pingServer && HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		isConnected = MONGO_OK == mongo_check_connection( this->conn );
	}

	return isConnected;
}

mongo* Helium::Mongo::Database::GetConnection()
{
	return &conn[0];
}

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