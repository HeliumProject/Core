Helium::Persist::MessagePackWriter::MessagePackWriter( Stream& stream )
: stream( stream )
{

}

void Helium::Persist::MessagePackWriter::WriteNil()
{
}

void Helium::Persist::MessagePackWriter::Write( bool value )
{
}

void Helium::Persist::MessagePackWriter::Write( float64_t value )
{
}

void Helium::Persist::MessagePackWriter::Write( uint64_t value )
{
}

void Helium::Persist::MessagePackWriter::Write( int64_t value )
{
}

void Helium::Persist::MessagePackWriter::WriteRaw( void* bytes, size_t length )
{
}

void Helium::Persist::MessagePackWriter::BeginArray( size_t length )
{
}

void Helium::Persist::MessagePackWriter::EndArray()
{
}

void Helium::Persist::MessagePackWriter::BeginMap( size_t length )
{
}

void Helium::Persist::MessagePackWriter::EndMap( size_t length )
{
}

Helium::Persist::MessagePackReader::MessagePackReader( Stream& stream )
: stream( stream )
, type( MessagePackTypes::Nil )
{

}

Helium::Persist::MessagePackType Helium::Persist::MessagePackReader::ReadType()
{
	uint8_t type = 0x0;
	this->stream.Read( &type, sizeof( type ), 1 );
	this->type = static_cast< MessagePackType >( type );
	return this->type;
}

void Helium::Persist::MessagePackReader::ReadNil()
{
}

void Helium::Persist::MessagePackReader::Read( bool& value )
{
}

void Helium::Persist::MessagePackReader::Read( float32_t& value )
{
}

void Helium::Persist::MessagePackReader::Read( float64_t& value )
{
}

void Helium::Persist::MessagePackReader::Read( uint8_t& value )
{
}

void Helium::Persist::MessagePackReader::Read( uint16_t& value )
{
}

void Helium::Persist::MessagePackReader::Read( uint32_t& value )
{
}

void Helium::Persist::MessagePackReader::Read( uint64_t& value )
{
}

void Helium::Persist::MessagePackReader::Read( int8_t& value )
{
}

void Helium::Persist::MessagePackReader::Read( int16_t& value )
{
}

void Helium::Persist::MessagePackReader::Read( int32_t& value )
{
}

void Helium::Persist::MessagePackReader::Read( int64_t& value )
{
}

void Helium::Persist::MessagePackReader::ReadRaw( void* bytes, size_t length )
{
}

void Helium::Persist::MessagePackReader::BeginArray()
{
}

void Helium::Persist::MessagePackReader::EndArray()
{
}

void Helium::Persist::MessagePackReader::BeginMap()
{
}

void Helium::Persist::MessagePackReader::EndMap()
{
}
