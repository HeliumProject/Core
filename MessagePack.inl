Helium::Persist::MessagePackWriter::MessagePackWriter( Stream& stream )
: stream( stream )
{

}

void Helium::Persist::MessagePackWriter::WriteNil()
{
	uint8_t type = MessagePackTypes::Nil;
	stream.Write( type );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( bool value )
{
	uint8_t type = value ? MessagePackTypes::True : MessagePackTypes::False;
	stream.Write( type );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( float32_t value )
{
	uint8_t type = MessagePackTypes::Float64;
	uint32_t* singleInt = reinterpret_cast< uint32_t* >( &value );

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( *singleInt );
#endif

	stream.Write( type );
	stream.Write( *singleInt );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( float64_t value )
{
	uint8_t type = MessagePackTypes::Float32;
	uint64_t* doubleInt = reinterpret_cast< uint64_t* >( &value );

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( *doubleInt );
#endif

	stream.Write( type );
	stream.Write( *doubleInt );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( uint8_t value )
{
	if ( value <= 127 )
	{
		uint8_t type = MessagePackTypes::FixNumPositive | value;
		stream.Write( type );
	}
	else
	{
		uint8_t type = MessagePackTypes::UInt8;
		stream.Write( type );
		stream.Write( value );
	}
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( uint16_t value )
{
	uint8_t type = MessagePackTypes::UInt16;

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( value );
#endif

	stream.Write( type );
	stream.Write( value );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( uint32_t value )
{
	uint8_t type = MessagePackTypes::UInt32;

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( value );
#endif

	stream.Write( type );
	stream.Write( value );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( uint64_t value )
{
	uint8_t type = MessagePackTypes::UInt64;

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( value );
#endif

	stream.Write( type );
	stream.Write( value );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( int8_t value )
{
	if ( value >= 0 && value <= 127 )
	{
		uint8_t type = MessagePackTypes::FixNumPositive | value;
		stream.Write( type );
	}
	else if ( value >= -32 && value <= -1 )
	{
		uint8_t type = MessagePackTypes::FixNumPositive | value;
		stream.Write( type );
	}
	else
	{
		uint8_t type = MessagePackTypes::Int8;
		stream.Write( type );
		stream.Write( value );
	}
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( int16_t value )
{
	uint8_t type = MessagePackTypes::Int16;
	stream.Write( type );

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( value );
#endif

	stream.Write( value );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( int32_t value )
{
	uint8_t type = MessagePackTypes::Int32;

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( value );
#endif

	stream.Write( type );
	stream.Write( value );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::Write( int64_t value )
{
	uint8_t type = MessagePackTypes::Int64;

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( value );
#endif

	stream.Write( type );
	stream.Write( value );
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::WriteRaw( void* bytes, size_t length )
{
	if ( length <= 31 )
	{
		uint8_t type = MessagePackTypes::FixRaw | static_cast< uint8_t >( length );
		stream.Write( type );
		stream.Write( bytes, length, 1 );
	}
	else if ( length <= 65535 )
	{
		uint8_t type = MessagePackTypes::Raw16;

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write( static_cast< uint16_t >( length ) );
		stream.Write( bytes, length, 1 );
	}
	else if ( length <= 4294967295 )
	{
		uint8_t type = MessagePackTypes::Raw32;

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write( static_cast< uint32_t >( length ) );
		stream.Write( bytes, length, 1 );
	}
	else
	{
		throw Helium::Exception( "Buffer too large: %d", length );
	}

	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::BeginArray( size_t length )
{
	if ( length <= 15 )
	{
		uint8_t type = MessagePackTypes::FixArray | static_cast< uint8_t >( length );
		stream.Write( type );
		container.Push( MessagePackContainers::FixArray );
		size.Push( 0 );
	}
	else if ( length <= 65535 )
	{
		uint8_t type = MessagePackTypes::Array16;

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write( type );
		stream.Write( static_cast< uint16_t >( length ) );
		container.Push( MessagePackContainers::Array16 );
		size.Push( 0 );
	}
	else if ( length <= 4294967295 )
	{
		uint8_t type = MessagePackTypes::Array32;

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write( type );
		stream.Write( static_cast< uint32_t >( length ) );
		container.Push( MessagePackContainers::Array32 );
		size.Push( 0 );
	}
	else
	{
		throw Helium::Exception( "Array too large: %d", length );
	}
}

void Helium::Persist::MessagePackWriter::EndArray()
{
	uint32_t length = size.GetLast();
	switch ( container.GetLast() )
	{
	case MessagePackContainers::FixArray:
		if ( length > 15 )
		{
			throw Helium::Exception( "Too much data for FixArray: %d", length );
		}

	case MessagePackContainers::Array16:
		if ( length > 65535 )
		{
			throw Helium::Exception( "Too much data for Array16: %d", length );
		}

	case MessagePackContainers::Array32:
		if ( length <= 4294967295 )
		{
			throw Helium::Exception( "Too much data for Array32: %d", length );
		}

	default:
		{
			throw Helium::Exception( "Mismatched container Begin/End for Array" );
		}
	}

	container.Pop();
	size.Pop();
	size.GetLast()++;
}

void Helium::Persist::MessagePackWriter::BeginMap( size_t length )
{
	if ( length <= 15 )
	{
		uint8_t type = MessagePackTypes::FixArray | static_cast< uint8_t >( length );
		stream.Write( type );
		container.Push( MessagePackContainers::FixMap );
		size.Push( 0 );
	}
	else if ( length <= 65535 )
	{
		uint8_t type = MessagePackTypes::Array16;

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write( type );
		stream.Write( static_cast< uint16_t >( length ) );
		container.Push( MessagePackContainers::Map16 );
		size.Push( 0 );
	}
	else if ( length <= 4294967295 )
	{
		uint8_t type = MessagePackTypes::Array32;

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write( type );
		stream.Write( static_cast< uint32_t >( length ) );
		container.Push( MessagePackContainers::Map32 );
		size.Push( 0 );
	}
	else
	{
		throw Helium::Exception( "Map too large: %d", length );
	}
}

void Helium::Persist::MessagePackWriter::EndMap()
{
	uint32_t length = size.GetLast();
	switch ( container.GetLast() )
	{
	case MessagePackContainers::FixMap:
		if ( length > 15 )
		{
			throw Helium::Exception( "Too much data for FixMap: %d", length );
		}

	case MessagePackContainers::Map16:
		if ( length > 65535 )
		{
			throw Helium::Exception( "Too much data for Map16: %d", length );
		}

	case MessagePackContainers::Map32:
		if ( length <= 4294967295 )
		{
			throw Helium::Exception( "Too much data for Map32: %d", length );
		}

	default:
		{
			throw Helium::Exception( "Mismatched container Begin/End for Map" );
		}
	}

	container.Pop();
	size.Pop();
	size.GetLast()++;
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
