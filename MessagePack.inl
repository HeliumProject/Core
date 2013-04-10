Helium::Persist::MessagePackWriter::MessagePackWriter( Stream& stream )
: stream( stream )
{

}

Helium::Persist::MessagePackReader::MessagePackReader( Stream& stream )
: stream( stream )
, type( MessagePackTypes::Nil )
{

}

uint8_t Helium::Persist::MessagePackReader::Advance()
{
	uint8_t type = 0x0;
	stream.Read( &type, sizeof( type ), 1 );
	this->type = type;
	return this->type;
}

void Helium::Persist::MessagePackReader::Skip()
{
	uint32_t length = 0x0;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) != MessagePackTypes::FixNumPositive
		&& ( type & MessagePackMasks::FixNumNegativeType ) != MessagePackTypes::FixNumNegative
		&& type != MessagePackTypes::Nil
		&& type != MessagePackTypes::False
		&& type != MessagePackTypes::True )
	{
		if ( ( type & MessagePackMasks::FixRawType ) == MessagePackTypes::FixRaw )
		{
			length = type & MessagePackMasks::FixRawCount;
		}
		else
		{
			if ( ( type & MessagePackMasks::FixArrayType ) == MessagePackTypes::FixArray )
			{
				uint32_t count = type & MessagePackMasks::FixArrayCount;
				for ( uint32_t i=0; i<count; ++i )
				{
					Advance();
					Skip();
				}
			}
			else
			{
				if ( ( type & MessagePackMasks::FixMapType ) == MessagePackTypes::FixMap )
				{
					uint32_t count = type & MessagePackMasks::FixMapCount;
					for ( uint32_t i=0; i<count; ++i )
					{
						Advance();
						Skip();
						Advance();
						Skip();
					}
				}
				else
				{
					switch ( type )
					{
					case MessagePackTypes::UInt8:
					case MessagePackTypes::Int8:
						length = 1;
						break;

					case MessagePackTypes::UInt16:
					case MessagePackTypes::Int16:
						length = 2;
						break;

					case MessagePackTypes::Float32:
					case MessagePackTypes::UInt32:
					case MessagePackTypes::Int32:
						length = 4;
						break;

					case MessagePackTypes::Float64:
					case MessagePackTypes::UInt64:
					case MessagePackTypes::Int64:
						length = 8;
						break;

					case MessagePackTypes::Raw16:
					case MessagePackTypes::Raw32:
						length = ReadRawLength();
						break;

					case MessagePackTypes::Array16:
					case MessagePackTypes::Array32:
						{
							uint32_t arrayLength = ReadArrayLength();
							for ( uint32_t i=0; i<arrayLength; ++i )
							{
								Advance();
								Skip();
							}
						}

					case MessagePackTypes::Map16:
					case MessagePackTypes::Map32:
						{
							uint32_t mapLength = ReadMapLength();
							for ( uint32_t i=0; i<mapLength; ++i )
							{
								Advance();
								Skip();
								Advance();
								Skip();
							}
						}
					}
				}
			}
		}
	}

	if ( length )
	{
		stream.Seek( length, SeekOrigins::Current );
	}
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

uint32_t Helium::Persist::MessagePackReader::ReadRawLength()
{
	uint32_t length = 0;

	if ( type & MessagePackMasks::FixRawType )
	{
		length = type & MessagePackMasks::FixRawCount;
	}
	else
	{
		switch ( type )
		{
		case MessagePackTypes::Raw16:
			{
				uint16_t temp;
				stream.Read< uint16_t >( temp );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( temp );
#endif
				length = temp;
				break;
			}

		case MessagePackTypes::Raw32:
			{
				uint32_t temp;
				stream.Read< uint32_t >( temp );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( temp );
#endif
				length = temp;
				break;
			}

		default:
			{
				throw Persist::Exception( "Object type is not a raw" );
			}
		}
	}

	return length;
}

uint32_t Helium::Persist::MessagePackReader::ReadArrayLength()
{
	uint32_t length = 0;

	if ( type & MessagePackMasks::FixArrayType )
	{
		length = type & MessagePackMasks::FixArrayCount;
	}
	else
	{
		switch ( type )
		{
		case MessagePackTypes::Array16:
			{
				uint16_t temp;
				stream.Read< uint16_t >( temp );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( temp );
#endif
				length = temp;
				break;
			}

		case MessagePackTypes::Array32:
			{
				uint32_t temp;
				stream.Read< uint32_t >( temp );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( temp );
#endif
				length = temp;
				break;
			}

		default:
			{
				throw Persist::Exception( "Object type is not an array" );
			}
		}
	}

	return length;
}

uint32_t Helium::Persist::MessagePackReader::ReadMapLength()
{
	uint32_t length = 0;

	if ( type & MessagePackMasks::FixMapType )
	{
		length = type & MessagePackMasks::FixMapCount;
	}
	else
	{
		switch ( type )
		{
		case MessagePackTypes::Map16:
			{
				uint16_t temp;
				stream.Read< uint16_t >( temp );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( temp );
#endif
				length = temp;
				break;
			}

		case MessagePackTypes::Map32:
			{
				uint32_t temp;
				stream.Read< uint32_t >( temp );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( temp );
#endif
				length = temp;
				break;
			}

		default:
			{
				throw Persist::Exception( "Object type is not a map" );
			}
		}
	}

	return length;
}
