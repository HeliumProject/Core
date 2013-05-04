#pragma once

#include "Foundation/DynamicArray.h"
#include "Foundation/FilePath.h"
#include "Foundation/MessagePack.h"
#include "Foundation/Stream.h"

#include "Persist/Archive.h"

namespace Helium
{
	namespace Persist
	{
		class HELIUM_PERSIST_API ArchiveWriterMessagePack : public ArchiveWriter
		{
		public:
			ArchiveWriterMessagePack( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL );
			ArchiveWriterMessagePack( Stream *stream, Reflect::ObjectIdentifier* identifier = NULL );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Write() HELIUM_OVERRIDE;

		private:
			void SerializeInstance( void* instance, const Reflect::Structure* structure, Reflect::Object* object );
			void SerializeField( void* instance, const Reflect::Field* field, Reflect::Object* object );
			void SerializeData( Reflect::DataPointer pointer, Reflect::Data* data, const Reflect::Field* field, Reflect::Object* object );

		public:
			static void ToStream( Reflect::Object* object, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );

		private:
			AutoPtr< Stream > m_Stream;
			MessagePackWriter m_Writer;
		};

		class HELIUM_PERSIST_API ArchiveReaderMessagePack : public ArchiveReader
		{
		public:
			ArchiveReaderMessagePack( const FilePath& path, Reflect::ObjectResolver* resolver = NULL );
			ArchiveReaderMessagePack( Stream *stream, Reflect::ObjectResolver* resolver = NULL );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Read() HELIUM_OVERRIDE;

		private:
			void Deserialize( DynamicArray< Reflect::ObjectPtr >& objects );
			void DeserializeInstance( void* instance, const Reflect::Structure* composite, Reflect::Object* object );
			void DeserializeField( void* instance, const Reflect::Field* field, Reflect::Object* object );
			void DeserializeData( Reflect::DataPointer pointer, Reflect::Data* data, const Reflect::Field* field, Reflect::Object* object );

		public:
			static Reflect::ObjectPtr FromStream( Stream& stream, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );

		private:
			AutoPtr< Stream > m_Stream;
			MessagePackReader m_Reader;
			int64_t           m_Size;
		};
	}
}