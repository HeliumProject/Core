Helium::MessagePackWriter::MessagePackWriter( Stream* stream )
: stream( stream )
{

}

void Helium::MessagePackWriter::SetStream( Stream* stream )
{
	if ( this->stream != stream )
	{
		this->stream = stream;
		this->containerState.Clear();
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
		this->containerState.Clear();
	}
}

void Helium::MessagePackReader::Advance()
{
	stream->Read( &type, sizeof( type ), 1 );
}

bool Helium::MessagePackReader::IsNil()
{
	return type == MessagePackTypes::Nil;
}

bool Helium::MessagePackReader::IsBoolean()
{
	return type == MessagePackTypes::False || type == MessagePackTypes::True;
}

bool Helium::MessagePackReader::IsNumber()
{
	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumPositive )
	{
		return true;
	}

	if ( ( type & MessagePackMasks::FixNumNegativeType ) == MessagePackTypes::FixNumNegative )
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
	if ( ( type & MessagePackMasks::FixRawType ) == MessagePackTypes::FixRaw )
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
	if ( ( type & MessagePackMasks::FixArrayType ) == MessagePackTypes::FixArray )
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
	if ( ( type & MessagePackMasks::FixMapType ) == MessagePackTypes::FixMap )
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
void Helium::MessagePackReader::ReadNumber( T& value, bool clamp, bool* succeeded )
{
	bool result = false;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumPositive )
	{
		value = type;
		result = true;

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
		}
	}
	else
	{
		if ( ( type & MessagePackMasks::FixNumNegativeType ) == MessagePackTypes::FixNumNegative )
		{
			value = static_cast< int8_t >( type );
			result = true;

			if ( !containerState.IsEmpty() )
			{
				containerState.GetLast().length--;
			}
		}
	}

	if ( !result )
	{
		switch ( type )
		{
		case MessagePackTypes::Float32:
		case MessagePackTypes::Float64:
			{
				float64_t temp = 0.0;
				ReadFloat( temp );
				result = RangeCast( temp, value, clamp );
				break;
			}

		case MessagePackTypes::UInt8:
		case MessagePackTypes::UInt16:
		case MessagePackTypes::UInt32:
		case MessagePackTypes::UInt64:
			{
				uint64_t temp = 0;
				ReadUnsigned( temp );
				result = RangeCast( temp, value, clamp );
				break;
			}

		case MessagePackTypes::Int8:
		case MessagePackTypes::Int16:
		case MessagePackTypes::Int32:
		case MessagePackTypes::Int64:
			{
				int64_t temp = 0;
				ReadSigned( temp );
				result = RangeCast( temp, value, clamp );
				break;
			}

		default:
			break;
		}
	}

	if ( succeeded )
	{
		*succeeded = result;
	}
	else if ( !result )
	{
		throw Helium::Exception( "Type mismatch on unhandled Read" );
	}
}
