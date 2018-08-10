#pragma once

#include "API.h"

#include "Platform/Utility.h"

#include "Foundation/DynamicArray.h"
#include "Foundation/FilePath.h"
#include "Foundation/Stream.h"

#include "Persist/Archive.h"

#define MONGO_HAVE_STDINT 1
#define MONGO_STATIC_BUILD 1
#include <bson.h>

namespace Helium
{
	namespace Persist
	{
		const char* GetBsonErrorString( int status );

		struct HELIUM_MONGO_API BsonDate : Helium::Reflect::Struct
		{
			int64_t millis; // milliseconds since epoch UTC

			inline BsonDate();

			inline bool operator==( const BsonDate& rhs ) const;
			inline bool operator!=( const BsonDate& rhs ) const;
			inline bool operator<( const BsonDate& rhs ) const;
			inline bool operator>( const BsonDate& rhs ) const;

			static BsonDate Now();

			HELIUM_DECLARE_BASE_STRUCT( BsonDate );
			static void PopulateMetaType( Helium::Reflect::MetaStruct& structure );
		};
		HELIUM_COMPILE_ASSERT( sizeof( bson_date_t ) == 8 );

		struct HELIUM_MONGO_API BsonObjectId : Helium::Reflect::Struct
		{
			uint8_t bytes[12]; // uint32_t timestamp, uint8_t machine[3], uint8_t process[2], uint8_t counter[3]

			inline BsonObjectId();
			inline BsonObjectId( const BsonObjectId& rhs );

			inline bool operator==( const BsonObjectId& rhs ) const;
			inline bool operator!=( const BsonObjectId& rhs ) const;
			inline bool operator<( const BsonObjectId& rhs ) const;
			inline bool operator>( const BsonObjectId& rhs ) const;
			inline operator bool() const;

			void IntoString( String& str ) const;
			void FromString( const String& str );

			static BsonObjectId Null;

			HELIUM_DECLARE_BASE_STRUCT( BsonObjectId );
			static void PopulateMetaType( Helium::Reflect::MetaStruct& structure );
		};
		HELIUM_COMPILE_ASSERT( sizeof( bson_oid_t ) == 12 );

		namespace BsonGeoAxes
		{
			enum Enum
			{
				Longitude,
				Latitude,
			};
		}

		struct HELIUM_MONGO_API BsonGeoPoint : Helium::Reflect::Struct
		{
			String type;
			float32_t coordinates[2];

			BsonGeoPoint()
			{
				type = "Point";
				coordinates[ BsonGeoAxes::Longitude ] = 0.f;
				coordinates[ BsonGeoAxes::Latitude ] = 0.f;
			}

			HELIUM_DECLARE_BASE_STRUCT( BsonGeoPoint );
			static void PopulateMetaType( Helium::Reflect::MetaStruct& structure );
		};

		struct HELIUM_MONGO_API BsonGeoLineString : Helium::Reflect::Struct
		{
			String type;
			DynamicArray< DynamicArray< float32_t > > coordinates;

			BsonGeoLineString()
			{
				type = "LineString";
			}

			HELIUM_DECLARE_BASE_STRUCT( BsonGeoLineString );
			static void PopulateMetaType( Helium::Reflect::MetaStruct& structure );
		};

		struct HELIUM_MONGO_API BsonGeoPolygon : Helium::Reflect::Struct
		{
			String type;
			DynamicArray< DynamicArray< DynamicArray< float32_t > > > coordinates;

			BsonGeoPolygon()
			{
				type = "Polygon";
			}

			HELIUM_DECLARE_BASE_STRUCT( BsonGeoPolygon );
			static void PopulateMetaType( Helium::Reflect::MetaStruct& structure );
		};

		class HELIUM_MONGO_API ArchiveWriterBson : public ArchiveWriter
		{
		public:
			static void Startup();
			static void Shutdown();
			static SmartPtr< ArchiveWriter > AllocateWriter( const FilePath& path, Reflect::ObjectIdentifier* identifier );
			static void WriteToStream( const Reflect::ObjectPtr& object, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );
			static void WriteToStream( const Reflect::ObjectPtr* objects, size_t count, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );
			static void WriteToBson( const Reflect::ObjectPtr& object, bson* b, const char* name = NULL, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );

			ArchiveWriterBson( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			ArchiveWriterBson( Stream *stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			
			virtual void Open() override;
			virtual void Close() override; 

		protected:
			virtual void Write( const Reflect::ObjectPtr* objects, size_t count ) override;

		private:
			void SerializeInstance( bson* b, const char* name, void* instance, const Reflect::MetaStruct* structure, Reflect::Object* object );
			void SerializeField( bson* b, void* instance, const Reflect::Field* field, Reflect::Object* object );
			void SerializeTranslator( bson* b, const char* name, Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

			AutoPtr< Stream >     m_Stream;
		};

		class HELIUM_MONGO_API ArchiveReaderBson : public ArchiveReader
		{
		public:
			static void Startup();
			static void Shutdown();
			static SmartPtr< ArchiveReader > AllocateReader( const FilePath& path, Reflect::ObjectResolver* resolver );
			static void ReadFromStream( Stream& stream, Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );
			static void ReadFromStream( Stream& stream, DynamicArray< Reflect::ObjectPtr > & objects, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );
			static void ReadFromBson( bson_iterator* i, const Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );

			ArchiveReaderBson( const FilePath& path, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			ArchiveReaderBson( Stream *stream, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			
			virtual void Open() override;
			virtual void Close() override; 

		protected:
			virtual void Read( DynamicArray< Reflect::ObjectPtr >& objects ) override;

		private:
			void Start();
			bool ReadNext( Reflect::ObjectPtr &object, size_t index );
			void DeserializeInstance( bson_iterator* i, void* instance, const Reflect::MetaStruct* composite, Reflect::Object* object );
			void DeserializeField( bson_iterator* i, void* instance, const Reflect::Field* field, Reflect::Object* object );
			void DeserializeTranslator( bson_iterator* i, Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

			DynamicArray< uint8_t > m_Buffer;
			AutoPtr< Stream >       m_Stream;
			int64_t                 m_Size;
			bson                    m_Bson[1];
			bson_iterator           m_Next[1];
		};
	}
}

#include "Mongo/ArchiveBson.inl"
