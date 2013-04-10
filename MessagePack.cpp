#include "PersistPch.h"
#include "Persist/MessagePack.h"

using namespace Helium;
using namespace Helium::Persist;

//
// Writer
//

void MessagePackWriter::WriteNil()
{
	stream.Write< uint8_t >( MessagePackTypes::Nil );

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( bool value )
{
	stream.Write< uint8_t >( value ? MessagePackTypes::True : MessagePackTypes::False );

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( float32_t value )
{
	// endian swap in an integer register to avoid NaN conditions
	uint32_t* temp = reinterpret_cast< uint32_t* >( &value );

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( *temp );
#endif

	stream.Write< uint8_t >( MessagePackTypes::Float32 );
	stream.Write< uint32_t >( *temp );

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( float64_t value )
{
	// endian swap in an integer register to avoid NaN conditions
	uint64_t* temp = reinterpret_cast< uint64_t* >( &value );

#if HELIUM_ENDIAN_LITTLE
	ConvertEndian( *temp );
#endif

	stream.Write< uint8_t >( MessagePackTypes::Float64 );
	stream.Write< uint64_t >( *temp );

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( uint8_t value )
{
	if ( value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream.Write< uint8_t >( value );
	}
	else
	{
		stream.Write< uint8_t >( MessagePackTypes::UInt8 );
		stream.Write< uint8_t >( value );
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( uint16_t value )
{
	if ( value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream.Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint8_t >::Maximum )
	{
		stream.Write< uint8_t >( MessagePackTypes::UInt8 );
		stream.Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( value );
#endif

		stream.Write< uint8_t >( MessagePackTypes::UInt16 );
		stream.Write< uint16_t >( value );
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( uint32_t value )
{
	if ( value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream.Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint8_t >::Maximum )
	{
		stream.Write< uint8_t >( MessagePackTypes::UInt8 );
		stream.Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint16_t >::Maximum )
	{
		uint16_t temp = static_cast< uint16_t >( value );

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( temp );
#endif

		stream.Write< uint8_t >( MessagePackTypes::UInt16 );
		stream.Write< uint16_t >( temp );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( value );
#endif

		stream.Write< uint8_t >( MessagePackTypes::UInt32 );
		stream.Write< uint32_t >( value );
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( uint64_t value )
{
	if ( value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream.Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint8_t >::Maximum )
	{
		stream.Write< uint8_t >( MessagePackTypes::UInt8 );
		stream.Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint16_t >::Maximum )
	{
		uint16_t temp = static_cast< uint16_t >( value );

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( temp );
#endif

		stream.Write< uint8_t >( MessagePackTypes::UInt16 );
		stream.Write< uint16_t >( temp );
	}
	else if ( value <= NumericLimits< uint32_t >::Maximum )
	{
		uint32_t temp = static_cast< uint32_t >( value );

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( temp );
#endif

		stream.Write< uint8_t >( MessagePackTypes::UInt32 );
		stream.Write< uint32_t >( temp );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( value );
#endif

		stream.Write< uint8_t >( MessagePackTypes::UInt64 );
		stream.Write< uint64_t >( value );
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( int8_t value )
{
	if ( value >= 0 && value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream.Write< int8_t >( value );
	}
	else if ( value < 0 && value >= -32 )
	{
		stream.Write< int8_t >( value );
	}
	else
	{
		stream.Write< uint8_t >( MessagePackTypes::Int8 );
		stream.Write< int8_t >( value );
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( int16_t value )
{
	if ( value >= 0 && value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream.Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value < 0 && value >= -32 )
	{
		stream.Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int8_t >::Minimum && value <= NumericLimits< int8_t >::Maximum )
	{
		stream.Write< uint8_t >( MessagePackTypes::Int8 );
		stream.Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( value );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Int16 );
		stream.Write< int16_t >( value );
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( int32_t value )
{
	if ( value >= 0 && value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream.Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value < 0 && value >= -32 )
	{
		stream.Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int8_t >::Minimum && value <= NumericLimits< int8_t >::Maximum )
	{
		stream.Write< uint8_t >( MessagePackTypes::Int8 );
		stream.Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int16_t >::Minimum && value <= NumericLimits< int16_t >::Maximum )
	{
		int16_t temp = static_cast< int16_t >( value );

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( temp );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Int16 );
		stream.Write< int16_t >( temp );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( value );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Int32 );
		stream.Write< int32_t >( value );
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::Write( int64_t value )
{
	if ( value >= 0 && value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream.Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value < 0 && value >= -32 )
	{
		stream.Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int8_t >::Minimum && value <= NumericLimits< int8_t >::Maximum )
	{
		stream.Write< uint8_t >( MessagePackTypes::Int8 );
		stream.Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int16_t >::Minimum && value <= NumericLimits< int16_t >::Maximum )
	{
		int16_t temp = static_cast< int16_t >( value );

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( temp );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Int16 );
		stream.Write< int16_t >( temp );
	}
	else if ( value >= NumericLimits< int32_t >::Minimum && value <= NumericLimits< int32_t >::Maximum )
	{
		int32_t temp = static_cast< int32_t >( value );

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( temp );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Int32 );
		stream.Write< int32_t >( temp );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( value );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Int64 );
		stream.Write< int64_t >( value );
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::WriteRaw( void* bytes, uint32_t length )
{
	if ( length <= 31 )
	{
		stream.Write< uint8_t >( MessagePackTypes::FixRaw | static_cast< uint8_t >( length ) );
		stream.Write( bytes, length, 1 );
	}
	else if ( length <= 65535 )
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Raw16 );
		stream.Write< uint16_t >( static_cast< uint16_t >( length ) );
		stream.Write( bytes, length, 1 );
	}
	else if ( length <= 4294967295 )
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Raw32 );
		stream.Write< uint32_t >( length );
		stream.Write( bytes, length, 1 );
	}
	else
	{
		throw Helium::Exception( "Buffer too large: %d", length );
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::BeginArray( uint32_t length )
{
	if ( length <= 15 )
	{
		stream.Write< uint8_t >( MessagePackTypes::FixArray | static_cast< uint8_t >( length ) );
		container.Push( MessagePackContainers::FixArray );
		size.Push( length );
	}
	else if ( length <= 65535 )
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Array16 );
		stream.Write< uint16_t >( static_cast< uint16_t >( length ) );
		container.Push( MessagePackContainers::Array16 );
		size.Push( length );
	}
	else if ( length <= 4294967295 )
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Array32 );
		stream.Write< uint32_t >( length );
		container.Push( MessagePackContainers::Array32 );
		size.Push( length );
	}
	else
	{
		throw Helium::Exception( "Array too large: %d", length );
	}
}

void MessagePackWriter::EndArray()
{
	uint32_t length = size.GetLast();
	switch ( container.GetLast() )
	{
	case MessagePackContainers::FixArray:
	case MessagePackContainers::Array16:
	case MessagePackContainers::Array32:
		{
			if ( length != 0 )
			{
				throw Helium::Exception( "Incorrent number of objects written into array, off by %d", length );
			}
		}

	default:
		{
			throw Helium::Exception( "Mismatched container Begin/End for array" );
		}
	}

	container.Pop();
	size.Pop();

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

void MessagePackWriter::BeginMap( uint32_t length )
{
	if ( length <= 15 )
	{
		stream.Write< uint8_t >( MessagePackTypes::FixArray | static_cast< uint8_t >( length ) );
		container.Push( MessagePackContainers::FixMap );
		size.Push( length );
	}
	else if ( length <= 65535 )
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Array16 );
		stream.Write< uint16_t >( static_cast< uint16_t >( length ) );
		container.Push( MessagePackContainers::Map16 );
		size.Push( length );
	}
	else if ( length <= 4294967295 )
	{
#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( length );
#endif

		stream.Write< uint8_t >( MessagePackTypes::Array32 );
		stream.Write< uint32_t >( length );
		container.Push( MessagePackContainers::Map32 );
		size.Push( length );
	}
	else
	{
		throw Helium::Exception( "Map too large: %d", length );
	}
}

void MessagePackWriter::EndMap()
{
	uint32_t length = size.GetLast();
	switch ( container.GetLast() )
	{
	case MessagePackContainers::FixMap:
	case MessagePackContainers::Map16:
	case MessagePackContainers::Map32:
		{
			if ( length != 0 )
			{
				throw Helium::Exception( "Incorrent number of objects written into map, off by %d", length );
			}
		}

	default:
		{
			throw Helium::Exception( "Mismatched container Begin/End for map" );
		}
	}

	container.Pop();
	size.Pop();

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}
}

//
// Reader
//

void MessagePackReader::Skip()
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

bool MessagePackReader::Read( bool& value )
{
	bool result = false;

	switch ( type )
	{
	case MessagePackTypes::True:
		{
			value = true;
			result = true;
			break;
		}

	case MessagePackTypes::False:
		{
			value = false;
			result = true;
			break;
		}
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( float32_t& value )
{
	bool result = false;

	if ( type == MessagePackTypes::Float32 )
	{
		uint32_t temp = 0x0;
		stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( temp );
#endif

		value = *reinterpret_cast< float32_t* >( &temp );
		result = true;
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( float64_t& value )
{
	bool result = false;

	if ( type == MessagePackTypes::Float64 )
	{
		uint64_t temp = 0x0;
		stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
		ConvertEndian( temp );
#endif

		value = *reinterpret_cast< float64_t* >( &temp );
		result = true;
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( uint8_t& value )
{
	bool result = false;

	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( type == MessagePackTypes::UInt8 )
		{
			stream.Read( value );
			result = true;
		}
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( uint16_t& value )
{
	bool result = false;

	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		result = true;
	}
	else
	{
		switch ( type )
		{
		case MessagePackTypes::UInt8:
			{
				uint8_t temp;
				stream.Read( temp );
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt16:
			{
				stream.Read( value );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( value );
#endif
				result = true;
				break;
			}
		}
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( uint32_t& value )
{
	bool result = false;

	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		result = true;
	}
	else
	{
		switch ( type )
		{
		case MessagePackTypes::UInt8:
			{
				uint8_t temp;
				stream.Read( temp );
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt16:
			{
				uint16_t temp;
				stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( temp );
#endif
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt32:
			{
				stream.Read( value );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( value );
#endif
				result = true;
				break;
			}
		}
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( uint64_t& value )
{
	bool result = false;

	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		result = true;
	}
	else
	{
		switch ( type )
		{
		case MessagePackTypes::UInt8:
			{
				uint8_t temp;
				stream.Read( temp );
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt16:
			{
				uint16_t temp;
				stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( temp );
#endif
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt32:
			{
				uint32_t temp;
				stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( temp );
#endif
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt64:
			{
				stream.Read( value );

#if HELIUM_ENDIAN_LITTLE
				ConvertEndian( value );
#endif
				result = true;
				break;
			}
		}
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( int8_t& value )
{
	bool result = false;

	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( type & MessagePackMasks::FixNumNegativeType )
		{
			value = type;
			result = true;
		}
		else
		{
			if ( type == MessagePackTypes::Int8 )
			{
				stream.Read( value );
				result = true;
			}
		}
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( int16_t& value )
{
	bool result = false;

	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( type & MessagePackMasks::FixNumNegativeType )
		{
			value = type;
			result = true;
		}
		else
		{
			switch ( type )
			{
			case MessagePackTypes::Int8:
				{
					int8_t temp;
					stream.Read( temp );
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int16:
				{
					stream.Read( value );

#if HELIUM_ENDIAN_LITTLE
					ConvertEndian( value );
#endif
					result = true;
					break;
				}
			}
		}
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( int32_t& value )
{
	bool result = false;

	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( type & MessagePackMasks::FixNumNegativeType )
		{
			value = type;
			result = true;
		}
		else
		{
			switch ( type )
			{
			case MessagePackTypes::Int8:
				{
					int8_t temp;
					stream.Read( temp );
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int16:
				{
					int16_t temp;
					stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
					ConvertEndian( temp );
#endif
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int32:
				{
					stream.Read( value );

#if HELIUM_ENDIAN_LITTLE
					ConvertEndian( value );
#endif
					result = true;
					break;
				}
			}
		}
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

bool MessagePackReader::Read( int64_t& value )
{
	bool result = false;

	if ( type & MessagePackMasks::FixNumPositiveType )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( type & MessagePackMasks::FixNumNegativeType )
		{
			value = type;
			result = true;
		}
		else
		{
			switch ( type )
			{
			case MessagePackTypes::Int8:
				{
					int8_t temp;
					stream.Read( temp );
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int16:
				{
					int16_t temp;
					stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
					ConvertEndian( temp );
#endif
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int32:
				{
					int32_t temp;
					stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
					ConvertEndian( temp );
#endif
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int64:
				{
					stream.Read( value );

#if HELIUM_ENDIAN_LITTLE
					ConvertEndian( value );
#endif
					result = true;
					break;
				}
			}
		}
	}

	if ( !size.IsEmpty() )
	{
		size.GetLast()--;
	}

	return result;
}

uint32_t MessagePackReader::ReadRawLength()
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

void MessagePackReader::ReadRaw( void* bytes, uint32_t length )
{
	stream.Read( bytes, length, 1 );
}

uint32_t MessagePackReader::ReadArrayLength()
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

void MessagePackReader::BeginArray()
{
#pragma TODO( __FUNCTION__ )
}

void MessagePackReader::EndArray()
{
#pragma TODO( __FUNCTION__ )
}

uint32_t MessagePackReader::ReadMapLength()
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

void MessagePackReader::BeginMap()
{
#pragma TODO( __FUNCTION__ )
}

void MessagePackReader::EndMap()
{
#pragma TODO( __FUNCTION__ )
}

void MessagePackReader::ReadFloat( float64_t& value )
{
	switch ( type )
	{
	case MessagePackTypes::Float32:
		{
			uint32_t temp = 0x0;
			stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
			ConvertEndian( temp );
#endif

			value = *reinterpret_cast< float32_t* >( &temp );
			break;
		}

	case MessagePackTypes::Float64:
		{
			uint64_t temp = 0x0;
			stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
			ConvertEndian( temp );
#endif

			value = *reinterpret_cast< float64_t* >( &temp );
			break;
		}

	default:
		{
			throw Persist::Exception( "Object type is not a float" );
		}
	}
}

void MessagePackReader::ReadUnsigned( uint64_t& value )
{
	switch ( type )
	{
	case MessagePackTypes::UInt8:
		{
			uint8_t temp = 0x0;
			stream.Read( temp );
			value = temp;
			break;
		}

	case MessagePackTypes::UInt16:
		{
			uint16_t temp = 0x0;
			stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
			ConvertEndian( temp );
#endif

			value = temp;
			break;
		}

	case MessagePackTypes::UInt32:
		{
			uint32_t temp = 0x0;
			stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
			ConvertEndian( temp );
#endif

			value = temp;
			break;
		}

	case MessagePackTypes::UInt64:
		{
			stream.Read( value );

#if HELIUM_ENDIAN_LITTLE
			ConvertEndian( value );
#endif
			break;
		}

	default:
		{
			throw Persist::Exception( "Object type is not an unsigned integer" );
		}
	}
}

void MessagePackReader::ReadSigned( int64_t& value )
{
	switch ( type )
	{
	case MessagePackTypes::Int8:
		{
			int8_t temp = 0x0;
			stream.Read( temp );
			value = temp;
			break;
		}

	case MessagePackTypes::Int16:
		{
			int16_t temp = 0x0;
			stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
			ConvertEndian( temp );
#endif

			value = temp;
			break;
		}

	case MessagePackTypes::Int32:
		{
			int32_t temp = 0x0;
			stream.Read( temp );

#if HELIUM_ENDIAN_LITTLE
			ConvertEndian( temp );
#endif

			value = temp;
			break;
		}

	case MessagePackTypes::Int64:
		{
			stream.Read( value );

#if HELIUM_ENDIAN_LITTLE
			ConvertEndian( value );
#endif
			break;
		}

	default:
		{
			throw Persist::Exception( "Object type is not a signed integer" );
		}
	}
}
