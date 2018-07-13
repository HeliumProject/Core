template< class T >
size_t Helium::Stream::Read( T& data )
{
	return this->Read( &data, sizeof( T ), 1 );
}

template< class T, size_t N >
size_t Helium::Stream::Read( T (&data)[N] )
{
	return this->Read( &data, sizeof( T ), N );
}

template< class T >
size_t Helium::Stream::Write( const T& data )
{
	return this->Write( &data, sizeof( T ), 1 );
}

template< class T, size_t N >
size_t Helium::Stream::Write( const T (&data)[N] )
{
	return this->Write( &data, sizeof( T ), N );
}