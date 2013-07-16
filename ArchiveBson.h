#pragma once

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
		class HELIUM_PERSIST_API ArchiveWriterBson : public ArchiveWriter
		{
		public:
			ArchiveWriterBson( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL );
			ArchiveWriterBson( Stream *stream, Reflect::ObjectIdentifier* identifier = NULL );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Write( Reflect::Object* object ) HELIUM_OVERRIDE;

		private:
			void SerializeInstance( bson* b, const char* name, void* instance, const Reflect::Structure* structure, Reflect::Object* object );
			void SerializeField( bson* b, void* instance, const Reflect::Field* field, Reflect::Object* object );
			void SerializeTranslator( bson* b, const char* name, Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

		public:
			static void ToStream( Reflect::Object* object, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );

		private:
			AutoPtr< Stream >     m_Stream;
		};

		class HELIUM_PERSIST_API ArchiveReaderBson : public ArchiveReader
		{
		public:
			ArchiveReaderBson( const FilePath& path, Reflect::ObjectResolver* resolver = NULL );
			ArchiveReaderBson( Stream *stream, Reflect::ObjectResolver* resolver = NULL );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Read( Reflect::ObjectPtr& object ) HELIUM_OVERRIDE;

			void Start();
			bool ReadNext( Reflect::ObjectPtr &object );
			void Resolve();

		private:
			void DeserializeInstance( bson_iterator* i, void* instance, const Reflect::Structure* composite, Reflect::Object* object );
			void DeserializeField( bson_iterator* i, void* instance, const Reflect::Field* field, Reflect::Object* object );
			void DeserializeTranslator( bson_iterator* i, Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

		public:
			static Reflect::ObjectPtr FromStream( Stream& stream, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );

		private:
			DynamicArray< uint8_t > m_Buffer;
			AutoPtr< Stream >       m_Stream;
			int64_t                 m_Size;
			bson                    m_Bson[1];
			bson_iterator           m_Next[1];
		};
	}
}
