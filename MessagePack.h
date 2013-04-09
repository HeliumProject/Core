#pragma once

#include "Foundation/DynamicArray.h"
#include "Foundation/Stream.h"
#include "Foundation/Endian.h"

#include "Persist/API.h"
#include "Persist/Exceptions.h"

namespace Helium
{
	namespace Persist
	{
		//
		// http://wiki.msgpack.org/display/MSGPACK/Format+specification
		//  This implementation is from the spec as of Feb 20, 2013
		//  All multi-byte values are big-endian with UTF-8 strings
		//

		namespace MessagePackTypes
		{
			enum Type
			{
				// Single byte objects
				FixNumPositive          = 0x00, // 0XXXXXXX
				FixNumNegative          = 0xe0, // 111XXXXX
				Nil                     = 0xc0,
				False                   = 0xc2,
				True                    = 0xc3,

				// Fixed size objects
				Float32                 = 0xca,
				Float64                 = 0xcb,
				UInt8                   = 0xcc,
				UInt16                  = 0xcd,
				UInt32                  = 0xce,
				UInt64                  = 0xcf,
				Int8                    = 0xd0,
				Int16                   = 0xd1,
				Int32                   = 0xd2,
				Int64                   = 0xd3,

				// Variable size objects
				FixRaw                  = 0xa0, // 101XXXXX
				Raw16                   = 0xda,
				Raw32                   = 0xdb,
				FixArray                = 0x90, // 1001XXXX
				Array16                 = 0xdc,
				Array32                 = 0xdd,
				FixMap                  = 0x80, // 1000XXXX
				Map16                   = 0xde,
				Map32                   = 0xdf,
			};
		};
		typedef MessagePackTypes::Type MessagePackType;

		namespace MessagePackMasks
		{
			enum Type
			{
				FixNumPositiveType      = 0x80, // 10000000
				FixNumPositiveValue     = 0x7f, // 01111111
				FixNumNegativeType      = 0xe0, // 11100000
				FixNumNegativeValue     = 0x1f, // 00011111
				FixRawType              = 0xe0, // 11100000
				FixRawCount             = 0x1f, // 00011111
				FixArrayType            = 0xf0, // 11110000
				FixArrayCount           = 0x0f, // 00001111
				FixMapType              = 0xf0, // 11110000
				FixMapCount             = 0x0f, // 00001111
			};
		};
		typedef MessagePackMasks::Type MessagePackMask;

		namespace MessagePackContainers
		{
			enum Type
			{
				FixArray,
				Array16,
				Array32,
				FixMap,
				Map16,
				Map32,
			};
		};
		typedef MessagePackContainers::Type MessagePackContainer;

		class HELIUM_PERSIST_API MessagePackWriter
		{
		public:
			inline MessagePackWriter( Stream& stream );

			inline void WriteNil();
			inline void Write( bool value );
			inline void Write( float32_t value );
			inline void Write( float64_t value );
			inline void Write( uint8_t value );
			inline void Write( uint16_t value );
			inline void Write( uint32_t value );
			inline void Write( uint64_t value );
			inline void Write( int8_t value );
			inline void Write( int16_t value );
			inline void Write( int32_t value );
			inline void Write( int64_t value );

			inline void WriteRaw( void* bytes, uint32_t length );

			inline void BeginArray( uint32_t length );
			inline void EndArray();
			
			inline void BeginMap( uint32_t length );
			inline void EndMap();

		private:
			Stream&                              stream;
			DynamicArray< MessagePackContainer > container; // for bookeeping container termination
			DynamicArray< uint32_t >             size;      // for bookeeping container size
		};

		class HELIUM_PERSIST_API MessagePackReader
		{
		public:
			inline MessagePackReader( Stream& stream );

			inline MessagePackType ReadType();

			inline void ReadNil();
			inline void Read( bool& value );
			inline void Read( float32_t& value );
			inline void Read( float64_t& value );
			inline void Read( uint8_t& value );
			inline void Read( uint16_t& value );
			inline void Read( uint32_t& value );
			inline void Read( uint64_t& value );
			inline void Read( int8_t& value );
			inline void Read( int16_t& value );
			inline void Read( int32_t& value );
			inline void Read( int64_t& value );

			inline void ReadRaw( void* bytes, uint32_t length );

			inline void BeginArray();
			inline void EndArray();
			
			inline void BeginMap();
			inline void EndMap();

		private:
			Stream&                              stream;
			MessagePackType                      type;
			DynamicArray< MessagePackContainer > container; // for bookeeping container termination
			DynamicArray< uint32_t >             size;      // for bookeeping container size
		};
	}
}

#include "Persist/MessagePack.inl"