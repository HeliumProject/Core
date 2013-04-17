Helium::MessagePackWriter::MessagePackWriter( Stream* stream )
: stream( stream )
{

}

void Helium::MessagePackWriter::SetStream( Stream* stream )
{
	if ( this->stream != stream )
	{
		this->stream = stream;
		this->container.Clear();
		this->size.Clear();
	}
}

Helium::MessagePackReader::MessagePackReader( Stream* stream )
: stream( stream )
, type( MessagePackTypes::Nil )
{

}

void Helium::MessagePackReader::SetStream( Stream* stream )
{
	if ( this->stream != stream )
	{
		this->stream = stream;
		this->type = MessagePackTypes::Nil;
		this->container.Clear();
		this->size.Clear();
	}
}

uint8_t Helium::MessagePackReader::Advance()
{
	uint8_t type = 0x0;
	stream->Read( &type, sizeof( type ), 1 );
	this->type = type;
	return this->type;
}

bool Helium::MessagePackReader::IsBoolean()
{
	return type == MessagePackTypes::False || type == MessagePackTypes::True;
}

bool Helium::MessagePackReader::IsNumber()
{
	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		return true;
	}

	if ( type & MessagePackMasks::FixNumNegativeType )
	{
		return true;
	}

	switch ( type )
	{
	case MessagePackTypes::Float32:
	case MessagePackTypes::Float64:
	case MessagePackTypes::UInt8:
	case MessagePackTypes::UInt16:
	case MessagePackTypes::UInt32:
	case MessagePackTypes::UInt64:
	case MessagePackTypes::Int8:
	case MessagePackTypes::Int16:
	case MessagePackTypes::Int32:
	case MessagePackTypes::Int64:
		return true;

	default:
		break;
	}

	return false;
}

bool Helium::MessagePackReader::IsRaw()
{
	if ( type & MessagePackMasks::FixRawType )
	{
		return true;
	}

	switch ( type )
	{
	case MessagePackTypes::Raw16:
	case MessagePackTypes::Raw32:
		{
			return true;
		}
	}

	return false;
}

bool Helium::MessagePackReader::IsArray()
{
	if ( type & MessagePackMasks::FixArrayType )
	{
		return true;
	}

	switch ( type )
	{
	case MessagePackTypes::Array16:
	case MessagePackTypes::Array32:
		{
			return true;
		}
	}

	return false;
}

bool Helium::MessagePackReader::IsMap()
{
	if ( type & MessagePackMasks::FixMapType )
	{
		return true;
	}

	switch ( type )
	{
	case MessagePackTypes::Map16:
	case MessagePackTypes::Map32:
		{
			return true;
		}
	}

	return false;
}

template< class T >
bool Helium::MessagePackReader::ReadNumber( T& value, bool clamp )
{
	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		return true;
	}

	if ( type & MessagePackMasks::FixNumNegativeType )
	{
		value = static_cast< int8_t >( type );
		return true;
	}

	switch ( type )
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
