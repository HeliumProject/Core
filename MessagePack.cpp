#include "Precompile.h"
#include "Foundation/MessagePack.h"

using namespace Helium;

//
// Writer
//

void MessagePackWriter::WriteNil()
{
	stream->Write< uint8_t >( MessagePackTypes::Nil );

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( bool value )
{
	stream->Write< uint8_t >( value ? MessagePackTypes::True : MessagePackTypes::False );

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( float32_t value )
{
	stream->Write< uint8_t >( MessagePackTypes::Float32 );

#if HELIUM_ENDIAN_LITTLE
	stream->Write< uint32_t >( ConvertEndianFloatToU32( value ) );
#else
	stream->Write< float32_t >( value );
#endif

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( float64_t value )
{
	stream->Write< uint8_t >( MessagePackTypes::Float64 );

#if HELIUM_ENDIAN_LITTLE
	stream->Write< uint64_t >( ConvertEndianDoubleToU64( value ) );
#else
	stream->Write< float64_t >( value );
#endif

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( uint8_t value )
{
	if ( value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream->Write< uint8_t >( value );
	}
	else
	{
		stream->Write< uint8_t >( MessagePackTypes::UInt8 );
		stream->Write< uint8_t >( value );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( uint16_t value )
{
	if ( value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream->Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint8_t >::Maximum )
	{
		stream->Write< uint8_t >( MessagePackTypes::UInt8 );
		stream->Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		value = ConvertEndian( value );
#endif
		stream->Write< uint8_t >( MessagePackTypes::UInt16 );
		stream->Write< uint16_t >( value );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( uint32_t value )
{
	if ( value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream->Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint8_t >::Maximum )
	{
		stream->Write< uint8_t >( MessagePackTypes::UInt8 );
		stream->Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint16_t >::Maximum )
	{
		uint16_t temp = static_cast< uint16_t >( value );
#if HELIUM_ENDIAN_LITTLE
		temp = ConvertEndian( temp );
#endif
		stream->Write< uint8_t >( MessagePackTypes::UInt16 );
		stream->Write< uint16_t >( temp );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		value = ConvertEndian( value );
#endif
		stream->Write< uint8_t >( MessagePackTypes::UInt32 );
		stream->Write< uint32_t >( value );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( uint64_t value )
{
	if ( value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream->Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint8_t >::Maximum )
	{
		stream->Write< uint8_t >( MessagePackTypes::UInt8 );
		stream->Write< uint8_t >( static_cast< uint8_t >( value ) );
	}
	else if ( value <= NumericLimits< uint16_t >::Maximum )
	{
		uint16_t temp = static_cast< uint16_t >( value );
#if HELIUM_ENDIAN_LITTLE
		temp = ConvertEndian( temp );
#endif
		stream->Write< uint8_t >( MessagePackTypes::UInt16 );
		stream->Write< uint16_t >( temp );
	}
	else if ( value <= NumericLimits< uint32_t >::Maximum )
	{
		uint32_t temp = static_cast< uint32_t >( value );
#if HELIUM_ENDIAN_LITTLE
		temp = ConvertEndian( temp );
#endif
		stream->Write< uint8_t >( MessagePackTypes::UInt32 );
		stream->Write< uint32_t >( temp );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		value = ConvertEndian( value );
#endif
		stream->Write< uint8_t >( MessagePackTypes::UInt64 );
		stream->Write< uint64_t >( value );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( int8_t value )
{
	if ( value >= 0 )
	{
		stream->Write< int8_t >( value );
	}
	else if ( value < 0 && value >= -32 )
	{
		stream->Write< int8_t >( value );
	}
	else
	{
		stream->Write< uint8_t >( MessagePackTypes::Int8 );
		stream->Write< int8_t >( value );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( int16_t value )
{
	if ( value >= 0 && value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream->Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value < 0 && value >= -32 )
	{
		stream->Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int8_t >::Minimum && value <= NumericLimits< int8_t >::Maximum )
	{
		stream->Write< uint8_t >( MessagePackTypes::Int8 );
		stream->Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		value = ConvertEndian( value );
#endif
		stream->Write< uint8_t >( MessagePackTypes::Int16 );
		stream->Write< int16_t >( value );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( int32_t value )
{
	if ( value >= 0 && value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream->Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value < 0 && value >= -32 )
	{
		stream->Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int8_t >::Minimum && value <= NumericLimits< int8_t >::Maximum )
	{
		stream->Write< uint8_t >( MessagePackTypes::Int8 );
		stream->Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int16_t >::Minimum && value <= NumericLimits< int16_t >::Maximum )
	{
		int16_t temp = static_cast< int16_t >( value );
#if HELIUM_ENDIAN_LITTLE
		temp = ConvertEndian( temp );
#endif
		stream->Write< uint8_t >( MessagePackTypes::Int16 );
		stream->Write< int16_t >( temp );
	}
	else
	{
#if HELIUM_ENDIAN_LITTLE
		value = ConvertEndian( value );
#endif
		stream->Write< uint8_t >( MessagePackTypes::Int32 );
		stream->Write< int32_t >( value );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( int64_t value )
{
	if ( value >= 0 && value <= MessagePackMasks::FixNumPositiveValue )
	{
		stream->Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value < 0 && value >= -32 )
	{
		stream->Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int8_t >::Minimum && value <= NumericLimits< int8_t >::Maximum )
	{
		stream->Write< uint8_t >( MessagePackTypes::Int8 );
		stream->Write< int8_t >( static_cast< int8_t >( value ) );
	}
	else if ( value >= NumericLimits< int16_t >::Minimum && value <= NumericLimits< int16_t >::Maximum )
	{
		int16_t temp = static_cast< int16_t >( value );
#if HELIUM_ENDIAN_LITTLE
		temp = ConvertEndian( temp );
#endif
		stream->Write< uint8_t >( MessagePackTypes::Int16 );
		stream->Write< int16_t >( temp );
	}
	else if ( value >= NumericLimits< int32_t >::Minimum && value <= NumericLimits< int32_t >::Maximum )
	{
		int32_t temp = static_cast< int32_t >( value );
#if HELIUM_ENDIAN_LITTLE
		temp = ConvertEndian( temp );
#endif
		stream->Write< uint8_t >( MessagePackTypes::Int32 );
		stream->Write< int32_t >( temp );
	}
	else
	{
		int64_t temp = value;
#if HELIUM_ENDIAN_LITTLE
		temp = ConvertEndian( temp );
#endif
		stream->Write< uint8_t >( MessagePackTypes::Int64 );
		stream->Write< int64_t >( temp );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::Write( const char* str )
{
	size_t length = StringLength( str );
	WriteRaw( str, static_cast< uint32_t >( length ) );
}

void MessagePackWriter::WriteRaw( const void* bytes, uint32_t length )
{
	if ( length <= 31 )
	{
		stream->Write< uint8_t >( MessagePackTypes::FixRaw | static_cast< uint8_t >( length ) );
		stream->Write( bytes, length, 1 );
	}
	else if ( length <= 65535 )
	{
		uint16_t temp = length;
#if HELIUM_ENDIAN_LITTLE
		temp = ConvertEndian( temp );
#endif
		stream->Write< uint8_t >( MessagePackTypes::Raw16 );
		stream->Write< uint16_t >( temp );
		stream->Write( bytes, length, 1 );
	}
	else
	{
		uint32_t temp = length;
#if HELIUM_ENDIAN_LITTLE
		temp = ConvertEndian( length );
#endif
		stream->Write< uint8_t >( MessagePackTypes::Raw32 );
		stream->Write< uint32_t >( temp );
		stream->Write( bytes, length, 1 );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::BeginArray( uint32_t length )
{
	ContainerState state;
	state.container = MessagePackContainers::Array;
	state.length = length;

	if ( length == NumericLimits< uint32_t >::Maximum )
	{
		stream->Write< uint8_t >( MessagePackTypes::Array32 );
		state.lengthOffset = stream->Tell();
		stream->Write< uint32_t >( length );
	}
	else
	{
		state.lengthOffset = Invalid< int64_t >();

		if ( length <= 15 )
		{
			stream->Write< uint8_t >( MessagePackTypes::FixArray | static_cast< uint8_t >( length ) );
		}
		else if ( length <= 65535 )
		{
			uint16_t temp = length;
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			stream->Write< uint8_t >( MessagePackTypes::Array16 );
			stream->Write< uint16_t >( temp );
		}
		else
		{
			uint32_t temp = length;
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			stream->Write< uint8_t >( MessagePackTypes::Array32 );
			stream->Write< uint32_t >( temp );
		}
	}

	containerState.Push( state );
}

void MessagePackWriter::EndArray()
{
	ContainerState state = containerState.Pop();

	if ( state.container == MessagePackContainers::Array )
	{
		if ( state.lengthOffset != Invalid< int64_t >() )
		{
			stream->Seek( state.lengthOffset, SeekOrigins::Begin );

			uint32_t temp = NumericLimits< uint32_t >::Maximum - state.length;
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			stream->Write< uint32_t >( temp );

			stream->Seek( 0, SeekOrigins::End );
		}
		else
		{
			if ( state.length != 0 )
			{
				throw Helium::Exception( "Incorrent number of objects written into array, off by %d", state.length );
			}
		}
	}
	else
	{
		throw Helium::Exception( "Mismatched container Begin/End for array" );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackWriter::BeginMap( uint32_t length )
{
	ContainerState state;
	state.container = MessagePackContainers::Map;
	state.length = length;

	if ( length == NumericLimits< uint32_t >::Maximum )
	{
		stream->Write< uint8_t >( MessagePackTypes::Map32 );
		state.lengthOffset = stream->Tell();
		stream->Write< uint32_t >( length );
	}
	else
	{
		state.lengthOffset = Invalid< int64_t >();

		if ( length <= 15 )
		{
			stream->Write< uint8_t >( MessagePackTypes::FixMap | static_cast< uint8_t >( length ) );
		}
		else if ( length <= 65535 )
		{
			uint16_t temp = length;
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			stream->Write< uint8_t >( MessagePackTypes::Array16 );
			stream->Write< uint16_t >( temp );
		}
		else
		{
			uint32_t temp = length;
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			stream->Write< uint8_t >( MessagePackTypes::Array32 );
			stream->Write< uint32_t >( temp );
		}

		// our state is going to bookkeep the number written, but we need to write TWICE as many due to key+value
		state.length *= 2;
	}

	containerState.Push( state );
}

void MessagePackWriter::EndMap()
{
	ContainerState state = containerState.Pop();

	if ( state.container == MessagePackContainers::Map )
	{
		if ( state.lengthOffset != Invalid< int64_t >() )
		{
			stream->Seek( state.lengthOffset, SeekOrigins::Begin );

			uint32_t temp = NumericLimits< uint32_t >::Maximum - state.length;
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			stream->Write< uint32_t >( temp );

			stream->Seek( 0, SeekOrigins::End );
		}
		else
		{
			if ( state.length != 0 )
			{
				throw Helium::Exception( "Incorrent number of objects written into map, off by %d", state.length );
			}
		}
	}
	else
	{
		throw Helium::Exception( "Mismatched container Begin/End for map" );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
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
		stream->Seek( length, SeekOrigins::Current );
	}

	Advance();
}

void MessagePackReader::Read( bool& value, bool* succeeded )
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

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( float32_t& value, bool* succeeded )
{
	bool result = false;

	if ( type == MessagePackTypes::Float32 )
	{
#if HELIUM_ENDIAN_LITTLE
		uint32_t temp = 0x0;
		stream->Read< uint32_t >( temp );
		value = ConvertEndianU32ToFloat( temp );
#else
		stream->Read< float32_t >( value );
#endif

		result = true;
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( float64_t& value, bool* succeeded )
{
	bool result = false;

	switch ( type )
	{
	case MessagePackTypes::Float32:
		{
#if HELIUM_ENDIAN_LITTLE
			uint32_t temp = 0x0;
			stream->Read< uint32_t >( temp );
			value = ConvertEndianU32ToFloat( temp );
#else
			stream->Read< float32_t >( value );
#endif
			result = true;
			break;
		}

	case MessagePackTypes::Float64:
		{
#if HELIUM_ENDIAN_LITTLE
			uint64_t temp = 0x0;
			stream->Read< uint64_t >( temp );
			value = ConvertEndianU64ToDouble( temp );
#else
			stream->Read< float64_t >( value );
#endif
			result = true;
			break;
		}
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( uint8_t& value, bool* succeeded )
{
	bool result = false;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumPositive )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( type == MessagePackTypes::UInt8 )
		{
			stream->Read< uint8_t >( value );
			result = true;
		}
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( uint16_t& value, bool* succeeded )
{
	bool result = false;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumNegative )
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
				stream->Read< uint8_t >( temp );
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt16:
			{
				stream->Read< uint16_t >( value );
#if HELIUM_ENDIAN_LITTLE
				value = ConvertEndian( value );
#endif
				result = true;
				break;
			}
		}
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( uint32_t& value, bool* succeeded )
{
	bool result = false;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumPositive )
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
				stream->Read< uint8_t >( temp );
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt16:
			{
				uint16_t temp;
				stream->Read< uint16_t >( temp );
#if HELIUM_ENDIAN_LITTLE
				temp = ConvertEndian( temp );
#endif
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt32:
			{
				stream->Read< uint32_t >( value );
#if HELIUM_ENDIAN_LITTLE
				value = ConvertEndian( value );
#endif
				result = true;
				break;
			}
		}
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( uint64_t& value, bool* succeeded )
{
	bool result = false;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumPositive )
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
				stream->Read< uint8_t >( temp );
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt16:
			{
				uint16_t temp;
				stream->Read< uint16_t >( temp );
#if HELIUM_ENDIAN_LITTLE
				temp = ConvertEndian( temp );
#endif
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt32:
			{
				uint32_t temp;
				stream->Read< uint32_t >( temp );
#if HELIUM_ENDIAN_LITTLE
				temp = ConvertEndian( temp );
#endif
				value = temp;
				result = true;
				break;
			}

		case MessagePackTypes::UInt64:
			{
				stream->Read< uint64_t >( value );
#if HELIUM_ENDIAN_LITTLE
				value = ConvertEndian( value );
#endif
				result = true;
				break;
			}
		}
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( int8_t& value, bool* succeeded )
{
	bool result = false;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumPositive )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( ( type & MessagePackMasks::FixNumNegativeType ) == MessagePackTypes::FixNumNegative )
		{
			value = type;
			result = true;
		}
		else
		{
			if ( type == MessagePackTypes::Int8 )
			{
				stream->Read< int8_t >( value );
				result = true;
			}
		}
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( int16_t& value, bool* succeeded )
{
	bool result = false;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumPositive )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( ( type & MessagePackMasks::FixNumNegativeType ) == MessagePackTypes::FixNumNegative )
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
					stream->Read< int8_t >( temp );
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int16:
				{
					stream->Read< int16_t >( value );
#if HELIUM_ENDIAN_LITTLE
					value = ConvertEndian( value );
#endif
					result = true;
					break;
				}
			}
		}
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( int32_t& value, bool* succeeded )
{
	bool result = false;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumPositive )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( ( type & MessagePackMasks::FixNumNegativeType ) == MessagePackTypes::FixNumNegative )
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
					stream->Read< int8_t >( temp );
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int16:
				{
					int16_t temp;
					stream->Read< int16_t >( temp );
#if HELIUM_ENDIAN_LITTLE
					temp = ConvertEndian( temp );
#endif
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int32:
				{
					stream->Read< int32_t >( value );
#if HELIUM_ENDIAN_LITTLE
					value = ConvertEndian( value );
#endif
					result = true;
					break;
				}
			}
		}
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( int64_t& value, bool* succeeded )
{
	bool result = false;

	if ( ( type & MessagePackMasks::FixNumPositiveType ) == MessagePackTypes::FixNumPositive )
	{
		value = type;
		result = true;
	}
	else
	{
		if ( ( type & MessagePackMasks::FixNumNegativeType ) == MessagePackTypes::FixNumNegative )
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
					stream->Read< int8_t >( temp );
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int16:
				{
					int16_t temp;
					stream->Read< int16_t >( temp );
#if HELIUM_ENDIAN_LITTLE
					temp = ConvertEndian( temp );
#endif
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int32:
				{
					int32_t temp;
					stream->Read< int32_t >( temp );
#if HELIUM_ENDIAN_LITTLE
					temp = ConvertEndian( temp );
#endif
					value = temp;
					result = true;
					break;
				}

			case MessagePackTypes::Int64:
				{
					stream->Read< int64_t >( value );
#if HELIUM_ENDIAN_LITTLE
					value = ConvertEndian( value );
#endif
					result = true;
					break;
				}
			}
		}
	}

	if ( result )
	{
		Advance();

		if ( !containerState.IsEmpty() )
		{
			containerState.GetLast().length--;
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

void MessagePackReader::Read( String& value )
{
	uint32_t length = ReadRawLength();
	value.Resize( length + 1 );
	ReadRaw( &value.GetFirst(), length );
	value += '\0';
}

uint32_t MessagePackReader::ReadRawLength()
{
	uint32_t length = 0;

	if ( ( type & MessagePackMasks::FixRawType ) == MessagePackTypes::FixRaw )
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
				stream->Read< uint16_t >( temp );
#if HELIUM_ENDIAN_LITTLE
				temp = ConvertEndian( temp );
#endif
				length = temp;
				break;
			}

		case MessagePackTypes::Raw32:
			{
				stream->Read< uint32_t >( length );
#if HELIUM_ENDIAN_LITTLE
				length = ConvertEndian( length );
#endif
				break;
			}

		default:
			{
				throw Helium::Exception( "Object type is not a raw" );
			}
		}
	}

	// do not Advance() since the next byte is not a type byte

	return length;
}

void MessagePackReader::ReadRaw( void* bytes, uint32_t length )
{
	stream->Read( bytes, length, 1 );

	Advance();

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

uint32_t MessagePackReader::ReadArrayLength()
{
	uint32_t length = 0;

	if ( ( type & MessagePackMasks::FixArrayType ) == MessagePackTypes::FixArray )
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
				stream->Read< uint16_t >( temp );
#if HELIUM_ENDIAN_LITTLE
				temp = ConvertEndian( temp );
#endif
				length = temp;
				break;
			}

		case MessagePackTypes::Array32:
			{
				stream->Read< uint32_t >( length );
#if HELIUM_ENDIAN_LITTLE
				length = ConvertEndian( length );
#endif
				break;
			}

		default:
			{
				throw Helium::Exception( "Object type is not an array" );
			}
		}
	}

	Advance();

	return length;
}

void MessagePackReader::BeginArray( uint32_t length )
{
	ContainerState state;
	state.container = MessagePackContainers::Array;
	state.length = length;
	containerState.Push( state );
}

void MessagePackReader::EndArray()
{
	ContainerState state = containerState.Pop();

	if ( state.container == MessagePackContainers::Array )
	{
		if ( state.length != 0 )
		{
			throw Helium::Exception( "Incorrent number of objects read from array, off by %d", state.length );
		}
	}
	else
	{
		throw Helium::Exception( "Mismatched container Begin/End for array" );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

uint32_t MessagePackReader::ReadMapLength()
{
	uint32_t length = 0;

	if ( ( type & MessagePackMasks::FixMapType ) == MessagePackTypes::FixMap )
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
				stream->Read< uint16_t >( temp );
#if HELIUM_ENDIAN_LITTLE
				temp = ConvertEndian( temp );
#endif
				length = temp;
				break;
			}

		case MessagePackTypes::Map32:
			{
				stream->Read< uint32_t >( length );
#if HELIUM_ENDIAN_LITTLE
				length = ConvertEndian( length );
#endif
				break;
			}

		default:
			{
				throw Helium::Exception( "Object type is not a map" );
			}
		}
	}

	Advance();

	return length;
}

void MessagePackReader::BeginMap( uint32_t length )
{
	ContainerState state;
	state.container = MessagePackContainers::Map;
	state.length = length * 2;
	containerState.Push( state );
}

void MessagePackReader::EndMap()
{
	ContainerState state = containerState.Pop();

	if ( state.container == MessagePackContainers::Map )
	{
		if ( state.length != 0 )
		{
			throw Helium::Exception( "Incorrent number of objects read from map, off by %d", state.length / 2 );
		}
	}
	else
	{
		throw Helium::Exception( "Mismatched container Begin/End for map" );
	}

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackReader::ReadFloat( float64_t& value )
{
	switch ( type )
	{
	case MessagePackTypes::Float32:
		{
#if HELIUM_ENDIAN_LITTLE
			uint32_t temp = 0x0;
			stream->Read< uint32_t >( temp );
			value = ConvertEndianU32ToFloat( temp );
#else
			stream->Read< float32_t >( value );
#endif
			break;
		}

	case MessagePackTypes::Float64:
		{
#if HELIUM_ENDIAN_LITTLE
			uint64_t temp = 0x0;
			stream->Read< uint64_t >( temp );
			value = ConvertEndianU64ToDouble( temp );
#else
			stream->Read< float64_t >( value );
#endif
			break;
		}

	default:
		{
			throw Helium::Exception( "Object type is not a float" );
		}
	}

	Advance();

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackReader::ReadUnsigned( uint64_t& value )
{
	switch ( type )
	{
	case MessagePackTypes::UInt8:
		{
			uint8_t temp = 0x0;
			stream->Read< uint8_t >( temp );
			value = temp;
			break;
		}

	case MessagePackTypes::UInt16:
		{
			uint16_t temp = 0x0;
			stream->Read< uint16_t >( temp );
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			value = temp;
			break;
		}

	case MessagePackTypes::UInt32:
		{
			uint32_t temp = 0x0;
			stream->Read< uint32_t >( temp );
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			value = temp;
			break;
		}

	case MessagePackTypes::UInt64:
		{
			stream->Read< uint64_t >( value );
#if HELIUM_ENDIAN_LITTLE
			value = ConvertEndian( value );
#endif
			break;
		}

	default:
		{
			throw Helium::Exception( "Object type is not an unsigned integer" );
		}
	}

	Advance();

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}

void MessagePackReader::ReadSigned( int64_t& value )
{
	switch ( type )
	{
	case MessagePackTypes::Int8:
		{
			int8_t temp = 0x0;
			stream->Read< int8_t >( temp );
			value = temp;
			break;
		}

	case MessagePackTypes::Int16:
		{
			int16_t temp = 0x0;
			stream->Read< int16_t >( temp );
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			value = temp;
			break;
		}

	case MessagePackTypes::Int32:
		{
			int32_t temp = 0x0;
			stream->Read< int32_t >( temp );
#if HELIUM_ENDIAN_LITTLE
			temp = ConvertEndian( temp );
#endif
			value = temp;
			break;
		}

	case MessagePackTypes::Int64:
		{
			stream->Read< int64_t >( value );
#if HELIUM_ENDIAN_LITTLE
			value = ConvertEndian( value );
#endif
			break;
		}

	default:
		{
			throw Helium::Exception( "Object type is not a signed integer" );
		}
	}

	Advance();

	if ( !containerState.IsEmpty() )
	{
		containerState.GetLast().length--;
	}
}
