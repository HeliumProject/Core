Helium::Persist::MessagePackWriter::MessagePackWriter( Stream* stream )
: stream( stream )
{

}

void Helium::Persist::MessagePackWriter::SetStream( Stream* stream )
{
	if ( this->stream != stream )
	{
		this->stream = stream;
		this->container.Clear();
		this->size.Clear();
	}
}

Helium::Persist::MessagePackReader::MessagePackReader( Stream* stream )
: stream( stream )
, type( MessagePackTypes::Nil )
{

}

void Helium::Persist::MessagePackReader::SetStream( Stream* stream )
{
	if ( this->stream != stream )
	{
		this->stream = stream;
		this->type = MessagePackTypes::Nil;
		this->container.Clear();
		this->size.Clear();
	}
}

uint8_t Helium::Persist::MessagePackReader::Advance()
{
	uint8_t type = 0x0;
	stream->Read( &type, sizeof( type ), 1 );
	this->type = type;
	return this->type;
}

template< class T >
bool Helium::Persist::MessagePackReader::ReadNumber( T& value, bool clamp )
{
	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		return true;
	}

	if ( type & MessagePackMasks::FixNumNegativeType )
	{
		value = reinterpret_cast< int8_t >( type );
		return true;
	}

	switch ( switchType )
	{
	case MessagePackTypes::Float32:
	case MessagePackTypes::Float64:
		{
			float64_t temp = 0.0;
			ReadFloat( temp );
			return RangeCast( temp, value, clamp );
		}

	case MessagePackTypes::UInt8:
	case MessagePackTypes::UInt16:
	case MessagePackTypes::UInt32:
	case MessagePackTypes::UInt64:
		{
			uint64_t temp = 0;
			ReadUnsigned( temp );
			return RangeCast( temp, value, clamp );
		}

	case MessagePackTypes::Int8:
	case MessagePackTypes::Int16:
	case MessagePackTypes::Int32:
	case MessagePackTypes::Int64:
		{
			int64_t temp = 0;
			ReadSigned( temp );
			return RangeCast( temp, value, clamp );
		}

	default:
		{
			return false;
		}
	}
}
