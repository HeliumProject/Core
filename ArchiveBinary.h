#pragma once

#include "Foundation/DynamicArray.h"
#include "Foundation/FilePath.h"
#include "Foundation/Stream.h"

#include "Persist/Archive.h"

//  
//    Reflect Binary Format:
//  
//    struct Data
//    {
//      int32_t type;           // string pool index of the name of the data
//      byte[] data;            // serialized data
//    };
//  
//    struct Field
//    {
//      int32_t field_id;       // latent type field index (name crc)
//      Data ser;               // data instance data
//    };
//  
//    struct Object
//    {
//      int32_t type;           // string pool index of the name of the object
//      int32_t field_count;    // number of serialized fields
//      Field[] fields;         // field instance data
//      int32_t term;           // -1
//    };
//  
//    struct ObjectArray
//    {
//      int32_t count;          // count of contained objects
//      Object[] objects;       // object instance data
//      int32_t term;           // -1
//    };
//  
//    struct File
//    {
//      uint8_t byte_order;     // BOM
//      uint8_t encoding;       // character encoding
//      uint32_t version;       // file format version
//
//      ObjectArray objects;    // client objects
//    };
//

#define PERSIST_ARCHIVE_VERBOSE

namespace Helium
{
	namespace Persist
	{
		extern const uint32_t BINARY_CURRENT_VERSION;

		class HELIUM_PERSIST_API ArchiveWriterBinary : public ArchiveWriter
		{
		public:
			ArchiveWriterBinary( const FilePath& path, Reflect::ObjectIdentifier& identifier );
			ArchiveWriterBinary( Stream *stream, Reflect::ObjectIdentifier& identifier );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			void OpenStream( Stream* stream, bool cleanup );
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Write() HELIUM_OVERRIDE;

		private:
			void SerializeInstance( Reflect::Object* object );
			void SerializeInstance( void* structure, const Reflect::Structure* type );
			void SerializeFields( Reflect::Object* object );
			void SerializeFields( void* structure, const Reflect::Structure* type );
			void SerializeArray( const DynamicArray< Reflect::ObjectPtr >& objects, uint32_t flags = 0 );

		public:
			static void ToStream( Reflect::Object* object, Stream& stream, Reflect::ObjectIdentifier& identifier );

		private:
			friend class Archive;

			AutoPtr< Stream > m_Stream;

			struct WriteFields
			{
				int32_t         m_Count;
				std::streamoff  m_CountOffset;
			};
			DynamicArray<WriteFields> m_FieldStack;
		};

		class HELIUM_PERSIST_API ArchiveReaderBinary : public ArchiveReader
		{
		public:
			ArchiveReaderBinary( const FilePath& path, Reflect::ObjectResolver& resolver );
			ArchiveReaderBinary( Stream *stream, Reflect::ObjectResolver& resolver );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			void OpenStream( Stream* stream, bool cleanup );
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Read() HELIUM_OVERRIDE;

		private:
			void DeserializeInstance( Reflect::ObjectPtr& object );
			void DeserializeInstance( void* structure, const Reflect::Structure* type );
			void DeserializeFields( Reflect::Object* object );
			void DeserializeFields( void* object, const Reflect::Structure* type );
			void DeserializeArray( DynamicArray< Reflect::ObjectPtr >& objects, uint32_t flags = 0 );
			Reflect::ObjectPtr Allocate();

		public:
			static Reflect::ObjectPtr FromStream( Stream& stream, Reflect::ObjectResolver& resolver );

		private:
			friend class Archive;

			AutoPtr< Stream > m_Stream;
			uint32_t m_Version;
			int64_t m_Size;
		};
	}
}