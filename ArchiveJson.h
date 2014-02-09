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

#include "rapidjson/include/rapidjson/prettywriter.h"
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
		typedef rapidjson::PrettyWriter< RapidJsonOutputStream > RapidJsonPrettyWriter;

		class HELIUM_PERSIST_API ArchiveWriterJson : public ArchiveWriter
		{
		public:
			static void WriteToStream( const Reflect::ObjectPtr& object, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );
			static void WriteToStream( const Reflect::ObjectPtr* objects, size_t count, Stream& stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );
			static void WriteToJson( const Reflect::ObjectPtr& object, RapidJsonWriter& writer, const char* name = NULL, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0 );

			ArchiveWriterJson( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			ArchiveWriterJson( Stream *stream, Reflect::ObjectIdentifier* identifier = NULL, uint32_t flags = 0x0 );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE;

		protected:
			virtual void Write( const Reflect::ObjectPtr* objects, size_t count );

		private:
			void SerializeInstance( RapidJsonWriter& writer, void* instance, const Reflect::MetaStruct* structure, Reflect::Object* object );
			void SerializeField( RapidJsonWriter& writer, void* instance, const Reflect::Field* field, Reflect::Object* object );
			void SerializeTranslator( RapidJsonWriter& writer, Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

			AutoPtr< Stream >     m_Stream;
			RapidJsonOutputStream m_Output;
		};

		class HELIUM_PERSIST_API ArchiveReaderJson : public ArchiveReader
		{
		public:
			static void ReadFromStream( Stream& stream, Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );
			static void ReadFromStream( Stream& stream, DynamicArray< Reflect::ObjectPtr >& objects, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );
			static void ReadFromJson( rapidjson::Value& value, const Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0 );

			ArchiveReaderJson( const FilePath& path, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			ArchiveReaderJson( Stream *stream, Reflect::ObjectResolver* resolver = NULL, uint32_t flags = 0x0 );
			
			virtual ArchiveType GetType() const HELIUM_OVERRIDE;
			virtual void Open() HELIUM_OVERRIDE;
			virtual void Close() HELIUM_OVERRIDE;

		protected:
			virtual void Read( DynamicArray< Reflect::ObjectPtr >& objects ) HELIUM_OVERRIDE;

		private:
			void Start();
			bool ReadNext( Reflect::ObjectPtr &object, size_t index );
			void DeserializeInstance( rapidjson::Value& value, void* instance, const Reflect::MetaStruct* composite, Reflect::Object* object );
			void DeserializeField( rapidjson::Value& value, void* instance, const Reflect::Field* field, Reflect::Object* object );
			void DeserializeTranslator( rapidjson::Value& value, Reflect::Pointer pointer, Reflect::Translator* translator, const Reflect::Field* field, Reflect::Object* object );

			DynamicArray< uint8_t > m_Buffer;
			AutoPtr< Stream >       m_Stream;
			rapidjson::Document     m_Document;
			rapidjson::SizeType     m_Next;
			int64_t                 m_Size;
		};
	}
}

#include "Persist/ArchiveJson.inl"