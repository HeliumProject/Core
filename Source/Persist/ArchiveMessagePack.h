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
			static void Startup();
			static void Shutdown();
			static SmartPtr< ArchiveWriter > AllocateWriter( const FilePath& path, Reflect::ObjectIdentifier* identifier );
			static void WriteToStream( const Reflect::ObjectPtr& object, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );
			static void WriteToStream( const Reflect::ObjectPtr* objects, size_t count, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );

			ArchiveWriterMessagePack( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			ArchiveWriterMessagePack( Stream *stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			
			virtual void Open() override;
			virtual void Close() override; 

		protected:
			virtual void Write( const Reflect::ObjectPtr* objects, size_t count ) override;

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
			static void Startup();
			static void Shutdown();
			static SmartPtr< ArchiveReader > AllocateReader( const FilePath& path, Reflect::ObjectResolver* resolver );
			static void ReadFromStream( Stream& stream, Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );
			static void ReadFromStream( Stream& stream, DynamicArray< Reflect::ObjectPtr >& objects, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );

			ArchiveReaderMessagePack( const FilePath& path, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			ArchiveReaderMessagePack( Stream *stream, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			
			virtual void Open() override;
			virtual void Close() override; 

		protected:
			virtual void Read( DynamicArray< Reflect::ObjectPtr >& objects ) override;

		private:
			void Start();
			bool ReadNext( Reflect::ObjectPtr &object, size_t index );
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