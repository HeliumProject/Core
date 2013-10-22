#pragma once

#include "Platform/Utility.h"

#include "Foundation/DynamicArray.h"
#include "Foundation/FilePath.h"
#include "Foundation/Stream.h"

#include "Persist/Archive.h"

#define MONGO_HAVE_STDINT 1
#define MONGO_STATIC_BUILD 1
#include <mongo-c/src/bson.h>

namespace Helium
{
	namespace Persist
	{
		const char* GetBsonErrorString( int status );

		struct HELIUM_PERSIST_API BsonDate : Helium::Reflect::Struct
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

		struct HELIUM_PERSIST_API BsonObjectId : Helium::Reflect::Struct
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

		class HELIUM_PERSIST_API ArchiveWriterBson : public ArchiveWriter
		{
		public:
			static void WriteToStream( const Reflect::ObjectPtr& object, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );
			static void WriteToStream( const Reflect::ObjectPtr* objects, size_t count, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );
			static void WriteToBson( const Reflect::ObjectPtr& object, bson* b, const char* name = NULL, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );

			ArchiveWriterBson( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			ArchiveWriterBson( Stream *stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 

		protected:
			virtual void Write( const Reflect::ObjectPtr* objects, size_t count );

		private:
			void SerializeInstance( bson* b, const char* name, void* instance, const Reflect::MetaStruct* structure, Reflect::Object* object );
			void SerializeField( bson* b, void* instance, const Reflect::Field* field, Reflect::Object* object );
			void SerializeTranslator( bson* b, const char* name, Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

			AutoPtr< Stream >     m_Stream;
		};

		class HELIUM_PERSIST_API ArchiveReaderBson : public ArchiveReader
		{
		public:
			static void ReadFromStream( Stream& stream, Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );
			static void ReadFromStream( Stream& stream, DynamicArray< Reflect::ObjectPtr > & objects, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );
			static void ReadFromBson( bson_iterator* i, const Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );

			ArchiveReaderBson( const FilePath& path, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			ArchiveReaderBson( Stream *stream, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 

		protected:
			virtual void Read( DynamicArray< Reflect::ObjectPtr >& objects ) HELIUM_OVERRIDE;

		public: // TEMP, don't call these!
			void Start();
			bool ReadNext( Reflect::ObjectPtr &object, size_t index );
		private:
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

#include "Persist/ArchiveBson.inl"
