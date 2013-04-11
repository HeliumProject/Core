#pragma once

#include "Foundation/DynamicArray.h"
#include "Foundation/FilePath.h"
#include "Foundation/Stream.h"

#include "Persist/Archive.h"
#include "Persist/MessagePack.h"

#define PERSIST_ARCHIVE_VERBOSE

namespace Helium
{
	namespace Persist
	{
		typedef void (Reflect::Object::*ObjectNotify)( const Reflect::Field* );

		class HELIUM_PERSIST_API ArchiveWriterBinary : public ArchiveWriter
		{
		public:
			ArchiveWriterBinary( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL );
			ArchiveWriterBinary( Stream *stream, Reflect::ObjectIdentifier* identifier = NULL );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Write() HELIUM_OVERRIDE;

		private:
			void SerializeArray( const DynamicArray< Reflect::ObjectPtr >& objects, uint32_t flags = 0 );
			void SerializeInstance( void* instance, const Reflect::Composite* type, Reflect::Object* object );
			void SerializeFields( void* instance, const Reflect::Composite* type, Reflect::Object* object );
			void SerializeField( void* instance, const Reflect::Field* field, Reflect::Object* object );

		public:
			static void ToStream( Reflect::Object* object, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL );

		private:
			AutoPtr< Stream > m_Stream;
			MessagePackWriter m_Writer;
		};

		class HELIUM_PERSIST_API ArchiveReaderBinary : public ArchiveReader
		{
		public:
			ArchiveReaderBinary( const FilePath& path, Reflect::ObjectResolver* resolver = NULL );
			ArchiveReaderBinary( Stream *stream, Reflect::ObjectResolver* resolver = NULL );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Read() HELIUM_OVERRIDE;

		private:
			void DeserializeArray( DynamicArray< Reflect::ObjectPtr >& objects, uint32_t flags = 0 );
			void DeserializeInstance( void* instance, const Reflect::Composite* composite, Reflect::Object* object );
			void DeserializeFields( void* instance, const Reflect::Composite* composite, Reflect::Object* object );
			void DeserializeField( void* instance, const Reflect::Field* field, Reflect::Object* object );

		public:
			static Reflect::ObjectPtr FromStream( Stream& stream, Reflect::ObjectResolver* resolver = NULL );

		private:
			AutoPtr< Stream > m_Stream;
			MessagePackReader m_Reader;
			int64_t           m_Size;
		};
	}
}