bool Helium::Mongo::Cursor::IsValid()
{
	return db != NULL && cursor != NULL;
}
	
bool Helium::Mongo::Database::IsConnected( bool pingServer )
{
	if ( isConnected && pingServer && HELIUM_VERIFY_MSG( IsCorrectThread(), "Database access from improper thread" ) )
	{
		isConnected = MONGO_OK == mongo_check_connection( this->conn );
	}

	return isConnected;
}

void Helium::Mongo::Database::SetThread( Helium::ThreadId threadId )
{
	this->threadId = threadId;
}

bool Helium::Mongo::Database::IsCorrectThread()
{
	return this->threadId == Thread::GetCurrentId();
}