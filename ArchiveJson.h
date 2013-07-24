#pragma once

#include "Foundation/DynamicArray.h"
#include "Foundation/FilePath.h"
#include "Foundation/Stream.h"

#include "Persist/Archive.h"

#if HELIUM_ENDIAN_BIG
# define RAPIDJSON_ENDIAN RAPIDJSON_BIGENDIAN
#elif HELIUM_ENDIAN_LITTLE
# define RAPIDJSON_ENDIAN RAPIDJSON_LITTLEENDIAN
#else
# error Unknown endianness!
#endif

#if HELIUM_ASSERT_ENABLED
# define RAPIDJSON_ASSERT HELIUM_ASSERT
#endif

#define RAPIDJSON_STATIC_ASSERT HELIUM_COMPILE_ASSERT

#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/document.h"

namespace Helium
{
	namespace Persist
	{
		class RapidJsonOutputStream
		{
		public:
			inline RapidJsonOutputStream();
			inline void SetStream( Stream* stream );
			inline void Put( char c );
			inline void Flush();

		private:
			Stream* m_Stream;
		};
		typedef rapidjson::Writer< RapidJsonOutputStream > RapidJsonWriter;

		class HELIUM_PERSIST_API ArchiveWriterJson : public ArchiveWriter
		{
		public:
			ArchiveWriterJson( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL );
			ArchiveWriterJson( Stream *stream, Reflect::ObjectIdentifier* identifier = NULL );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Write( Reflect::Object* object ) HELIUM_OVERRIDE;

		private:
			void SerializeInstance( void* instance, const Reflect::MetaStruct* structure, Reflect::Object* object );
			void SerializeField( void* instance, const Reflect::Field* field, Reflect::Object* object );
			void SerializeTranslator( Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

		public:
			static void ToStream( Reflect::Object* object, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );

		private:
			AutoPtr< Stream >     m_Stream;
			RapidJsonOutputStream m_Output;
			RapidJsonWriter       m_Writer;
		};

		class HELIUM_PERSIST_API ArchiveReaderJson : public ArchiveReader
		{
		public:
			ArchiveReaderJson( const FilePath& path, Reflect::ObjectResolver* resolver = NULL );
			ArchiveReaderJson( Stream *stream, Reflect::ObjectResolver* resolver = NULL );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE; 
			virtual void Read( Reflect::ObjectPtr& object ) HELIUM_OVERRIDE;

			void Start();
			bool ReadNext( Reflect::ObjectPtr &object );
			void Resolve();

		private:
			void DeserializeInstance( rapidjson::Value& value, void* instance, const Reflect::MetaStruct* composite, Reflect::Object* object );
			void DeserializeField( rapidjson::Value& value, void* instance, const Reflect::Field* field, Reflect::Object* object );
			void DeserializeTranslator( rapidjson::Value& value, Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

		public:
			static Reflect::ObjectPtr FromStream( Stream& stream, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );

		private:
			DynamicArray< uint8_t > m_Buffer;
			AutoPtr< Stream >       m_Stream;
			rapidjson::Document     m_Document;
			rapidjson::SizeType     m_Next;
			int64_t                 m_Size;
		};
	}
}

#include "Persist/ArchiveJson.inl"