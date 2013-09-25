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
			static void WriteToStream( Reflect::Object* object, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );

			ArchiveWriterMessagePack( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			ArchiveWriterMessagePack( Stream *stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 

		protected:
			virtual void Write( Reflect::Object* object ) HELIUM_OVERRIDE;

		private:
			void SerializeInstance( void* instance, const Reflect::MetaStruct* structure, Reflect::Object* object );
			void SerializeField( void* instance, const Reflect::Field* field, Reflect::Object* object );
			void SerializeTranslator( Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

			AutoPtr< Stream > m_Stream;
			MessagePackWriter m_Writer;
		};

		class HELIUM_PERSIST_API ArchiveReaderMessagePack : public ArchiveReader
		{
		public:
			static void ReadFromStream( Stream& stream, Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );

			ArchiveReaderMessagePack( const FilePath& path, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			ArchiveReaderMessagePack( Stream *stream, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 

		protected:
			virtual void Read( Reflect::ObjectPtr& object ) HELIUM_OVERRIDE;

		public: // TEMP, don't call these!
			void Start();
			bool ReadNext( Reflect::ObjectPtr &object );
		private:
			void DeserializeInstance( void* instance, const Reflect::MetaStruct* composite, Reflect::Object* object );
			void DeserializeField( void* instance, const Reflect::Field* field, Reflect::Object* object );
			void DeserializeTranslator( Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

		private:
			AutoPtr< Stream > m_Stream;
			MessagePackReader m_Reader;
			int64_t           m_Size;
		};
	}
}