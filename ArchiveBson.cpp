#include "PersistPch.h"
#include "Persist/ArchiveBson.h"

#include "Foundation/Endian.h"
#include "Foundation/FileStream.h"
#include "Foundation/Numeric.h"

#include "Reflect/Object.h"
#include "Reflect/Structure.h"
#include "Reflect/Registry.h"
#include "Reflect/TranslatorDeduction.h"

#if 0

#include <bson/bson.h>

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;

ArchiveWriterBson::ArchiveWriterBson( const FilePath& path, ObjectIdentifier* identifier )
	: ArchiveWriter( path, identifier )
{
}

ArchiveWriterBson::ArchiveWriterBson( Stream *stream, ObjectIdentifier* identifier )
	: ArchiveWriter( identifier )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
}

ArchiveType ArchiveWriterBson::GetType() const
{
	return ArchiveTypes::Json;
}

void ArchiveWriterBson::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Opening file '%s'\n"), m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path, FileStream::MODE_WRITE );
	m_Stream.Reset( stream );
}

void ArchiveWriterBson::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close();
	m_Stream.Release();
}

void ArchiveWriterBson::Write( Object* object )
{
	PERSIST_SCOPE_TIMER( ("Reflect - Json Write") );

	// notify starting
	ArchiveStatus info( *this, ArchiveStates::Starting );
	e_Status.Raise( info );

	// the master object
	m_Objects.Push( object );

	mongo::BSONArrayBuilder arrayBuilder;

	// objects can get changed during this iteration (in Identify), so use indices
	for ( size_t index = 0; index < m_Objects.GetSize(); ++index )
	{
		Object* object = m_Objects.GetElement( index );
		const Class* objectClass = object->GetClass();

		mongo::BSONObjBuilder objBuilder;
		SerializeInstance( objBuilder, object, objectClass, object );
		arrayBuilder.append( BSON( objectClass->m_Name << objBuilder.obj() ) );

		info.m_State = ArchiveStates::ObjectProcessed;
		info.m_Progress = (int)(((float)(index) / (float)m_Objects.GetSize()) * 100.0f);
		e_Status.Raise( info );
	}

	m_Stream->Write( arrayBuilder.arr().objdata(), arrayBuilder.arr().objsize(), 1 );

	// notify completion of last object processed
	info.m_State = ArchiveStates::ObjectProcessed;
	info.m_Progress = 100;
	e_Status.Raise( info );

	// do cleanup
	m_Stream->Flush();

	// notify completion
	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveWriterBson::SerializeInstance( mongo::BSONObjBuilder& builder, void* instance, const Structure* structure, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print( TXT( "Serializing %s\n" ), structure->m_Name );
#endif

#pragma TODO("Declare a max depth for inheritance to save heap allocs -geoff")
	DynamicArray< const Structure* > bases;
	for ( const Structure* current = structure; current != NULL; current = current->m_Base )
	{
		bases.Push( current );
	}

#pragma TODO("Declare a max count for fields to save heap allocs -geoff")
	DynamicArray< const Field* > fields;
	while ( !bases.IsEmpty() )
	{
		const Structure* current = bases.Pop();
		DynamicArray< Field >::ConstIterator itr = current->m_Fields.Begin();
		DynamicArray< Field >::ConstIterator end = current->m_Fields.End();
		for ( ; itr != end; ++itr )
		{
			const Field* field = &*itr;
			if ( field->ShouldSerialize( instance, object ) )
			{
				fields.Push( field );
			}
		}
	}

	object->PreSerialize( NULL );

	DynamicArray< const Field* >::ConstIterator itr = fields.Begin();
	DynamicArray< const Field* >::ConstIterator end = fields.End();
	for ( ; itr != end; ++itr )
	{
		const Field* field = *itr;

		object->PreSerialize( field );

		SerializeField( builder, instance, field, object );

		object->PostSerialize( field );
	}

	object->PostSerialize( NULL );
}

void ArchiveWriterBson::SerializeField( mongo::BSONObjBuilder& builder, void* instance, const Field* field, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Serializing field %s\n"), field->m_Name);
#endif

	if ( field->m_Count > 1 )
	{
		mongo::BSONArrayBuilder arrayBuilder;

		for ( uint32_t i=0; i<field->m_Count; ++i )
		{
			mongo::BSONObjBuilder objBuilder;
			SerializeTranslator( objBuilder, Pointer ( field, instance, object, i ), field->m_Translator, field, object );
			arrayBuilder.appendAs( objBuilder.obj().firstElement(), mongo::BSONObjBuilder::numStr( arrayBuilder.arrSize() ) );
		}

		builder.append( field->m_Name, arrayBuilder.arr() );
	}
	else
	{
		mongo::BSONObjBuilder objBuilder;
		SerializeTranslator( objBuilder, Pointer ( field, instance, object ), field->m_Translator, field, object );
		builder.appendAs( objBuilder.obj().firstElement(), field->m_Name );
	}
}

void ArchiveWriterBson::SerializeTranslator( mongo::BSONObjBuilder& builder, Pointer pointer, Translator* translator, const Field* field, Object* object )
{
	switch ( translator->GetReflectionType() )
	{
	case ReflectionTypes::ScalarTranslator:
	case ReflectionTypes::EnumerationTranslator:
	case ReflectionTypes::PointerTranslator:
	case ReflectionTypes::TypeTranslator:
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			switch ( scalar->m_Type )
			{
			case ScalarTypes::Boolean:
				builder.append( "temp", pointer.As<bool>() );
				break;

			case ScalarTypes::Unsigned8:
				builder.append( "temp", pointer.As<uint8_t>() );
				break;

			case ScalarTypes::Unsigned16:
				builder.append( "temp", pointer.As<uint16_t>() );
				break;

			case ScalarTypes::Unsigned32:
				builder.append( "temp", pointer.As<uint32_t>() );
				break;

			case ScalarTypes::Unsigned64:
				builder.append( "temp", pointer.As<int64_t>() ); // uint64_t isn't supported, just hope for the best
				break;

			case ScalarTypes::Signed8:
				builder.append( "temp", pointer.As<int8_t>() );
				break;

			case ScalarTypes::Signed16:
				builder.append( "temp", pointer.As<int16_t>() );
				break;

			case ScalarTypes::Signed32:
				builder.append( "temp", pointer.As<int32_t>() );
				break;

			case ScalarTypes::Signed64:
				builder.append( "temp", pointer.As<int64_t>() );
				break;

			case ScalarTypes::Float32:
				builder.append( "temp", pointer.As<float32_t>() );
				break;

			case ScalarTypes::Float64:
				builder.append( "temp", pointer.As<float64_t>() );
				break;

			case ScalarTypes::String:
				String str;
				scalar->Print( pointer, str, this );
				builder.append( "temp", str.GetData() );
				break;
			}
			break;
		}

	case ReflectionTypes::StructureTranslator:
		{
			StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
			SerializeInstance( builder, pointer.m_Address, structure->GetStructure(), object );
			break;
		}

	case ReflectionTypes::SetTranslator:
		{
			SetTranslator* set = static_cast< SetTranslator* >( translator );

			Translator* itemTranslator = set->GetItemTranslator();
			DynamicArray< Pointer > items;
			set->GetItems( pointer, items );

			mongo::BSONArrayBuilder arrayBuilder;
			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr )
			{
				mongo::BSONObjBuilder objBuilder;
				SerializeTranslator( objBuilder, *itr, itemTranslator, field, object );
				arrayBuilder.appendAs( objBuilder.obj().firstElement(), mongo::BSONObjBuilder::numStr( arrayBuilder.arrSize() ) );
			}

			builder.append( "temp", arrayBuilder.arr() );
			break;
		}

	case ReflectionTypes::SequenceTranslator:
		{
			SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );

			Translator* itemTranslator = sequence->GetItemTranslator();
			DynamicArray< Pointer > items;
			sequence->GetItems( pointer, items );

			mongo::BSONArrayBuilder arrayBuilder;
			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr )
			{
				mongo::BSONObjBuilder objBuilder;
				SerializeTranslator( objBuilder, *itr, itemTranslator, field, object );
				arrayBuilder.appendAs( objBuilder.obj().firstElement(), mongo::BSONObjBuilder::numStr( arrayBuilder.arrSize() ) );
			}

			builder.append( "temp", arrayBuilder.arr() );
			break;
		}

	case ReflectionTypes::AssociationTranslator:
		{
			AssociationTranslator* association = static_cast< AssociationTranslator* >( translator );

			Translator* keyTranslator = association->GetKeyTranslator();
			Translator* valueTranslator = association->GetValueTranslator();
			DynamicArray< Pointer > keys, values;
			association->GetItems( pointer, keys, values );

			mongo::BSONObjBuilder objBuilder;
			for ( DynamicArray< Pointer >::Iterator keyItr = keys.Begin(), valueItr = values.Begin(), keyEnd = keys.End(), valueEnd = values.End();
				keyItr != keyEnd && valueItr != valueEnd;
				++keyItr, ++valueItr )
			{
				mongo::BSONObjBuilder keyObjBuilder;
				SerializeTranslator( keyObjBuilder, *keyItr, keyTranslator, field, object );

				mongo::BSONObjBuilder valueObjBuilder;
				SerializeTranslator( valueObjBuilder, *valueItr, valueTranslator, field, object );

				objBuilder.appendAs( valueObjBuilder.obj().firstElement(), keyObjBuilder.obj().firstElement().toString( false ) );
			}

			builder.append( "temp", objBuilder.obj() );
			break;
		}

	default:
		// Unhandled reflection type in ArchiveWriterBson::SerializeTranslator
		HELIUM_ASSERT_FALSE();
	}
}

void ArchiveWriterBson::ToStream( Object* object, Stream& stream, ObjectIdentifier* identifier, uint32_t flags )
{
	ArchiveWriterBson archive ( &stream, identifier );
	archive.m_Flags = flags;
	archive.Write( object );
	archive.Close();
}

ArchiveReaderBson::ArchiveReaderBson( const FilePath& path, ObjectResolver* resolver )
	: ArchiveReader( path, resolver )
	, m_Stream( NULL )
	, m_Size( 0 )
{

}

ArchiveReaderBson::ArchiveReaderBson( Stream *stream, ObjectResolver* resolver )
	: ArchiveReader( resolver )
	, m_Stream( NULL )
	, m_Size( 0 )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
}

ArchiveType ArchiveReaderBson::GetType() const
{
	return ArchiveTypes::Bson;
}

void ArchiveReaderBson::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Opening file '%s'\n"), m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path, FileStream::MODE_READ );
	m_Stream.Reset( stream );
}

void ArchiveReaderBson::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close();
	m_Stream.Release();
}

void ArchiveReaderBson::Read( Reflect::ObjectPtr& object )
{
	PERSIST_SCOPE_TIMER( ("Reflect - Json Read") );

	Start();

#if 0
	if ( m_Document.IsArray() )
	{
		uint32_t length = m_Document.Size();
		m_Objects.Reserve( length );
		for ( uint32_t i=0; i<length; i++ )
		{
			ObjectPtr object;
			ReadNext( object );

			ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
			info.m_Progress = (int)(((float)(m_Stream->Tell()) / (float)m_Size) * 100.0f);
			e_Status.Raise( info );
			m_Abort |= info.m_Abort;
			if ( m_Abort )
			{
				break;
			}
		}
	}
#endif

	Resolve();

	object = m_Objects.GetFirst();
}

void Helium::Persist::ArchiveReaderBson::Start()
{
#if 0
	ArchiveStatus info( *this, ArchiveStates::Starting );
	e_Status.Raise( info );
	m_Abort = false;

	// determine the size of the input stream
	m_Stream->Seek(0, SeekOrigins::End);
	m_Size = m_Stream->Tell();
	m_Stream->Seek(0, SeekOrigins::Begin);

	// fail on an empty input stream
	if ( m_Size == 0 )
	{
		throw Persist::StreamException( TXT( "Input stream is empty (%s)" ), m_Path.c_str() );
	}

	// read entire contents
	m_Buffer.Resize( static_cast< size_t >( m_Size + 1 ) );
	m_Stream->Read( m_Buffer.GetData(),  static_cast< size_t >( m_Size ), 1 );
	m_Buffer[ static_cast< size_t >( m_Size ) ] = '\0';
	if ( m_Document.ParseInsitu< 0 >( reinterpret_cast< char* >( m_Buffer.GetData() ) ).HasParseError() )
	{
		m_Stream->Seek( 0, SeekOrigins::Begin );
		size_t lineCount = 1;
		size_t charCount = 0;

		size_t i = 0;
		while ( true )
		{
			char c;
			if ( m_Stream->Read( &c, 1, 1 ) && i < m_Document.GetErrorOffset() )
			{
				if (c == '\n')
				{
					++lineCount;
					charCount = 0;
				}
				else
				{
					if (c != '\r')
					{
						++charCount;
					}
				}
			}
			else
			{
				break;
			}
			
			++i;
		}

		throw Persist::Exception( "Error parsing JSON (%d,%d): %s", lineCount, charCount, m_Document.GetParseError() );
	}
#endif
}

bool Helium::Persist::ArchiveReaderBson::ReadNext( Reflect::ObjectPtr& object )
{
	bool success = false;
#if 0
	if (m_Next >= m_Document.Size())
	{
		return false;
	}

	rapidjson::Value& value = m_Document[ m_Next++ ];
	if ( value.IsObject() )
	{
		rapidjson::Value::Member* member = value.MemberBegin();
		if ( member != value.MemberEnd() )
		{
			uint32_t objectClassCrc = 0;
			if ( member->name.IsString() )
			{
				String typeStr;
				typeStr = member->name.GetString();
				objectClassCrc = Helium::Crc32( typeStr.GetData() );
			}

			const Class* objectClass = NULL;
			if ( objectClassCrc != 0 )
			{
				objectClass = Registry::GetInstance()->GetClass( objectClassCrc );
			}
			
			if ( !object && objectClass )
			{
				object = objectClass->m_Creator();
			}

			if ( object.ReferencesObject() )
			{
				success = true;
				DeserializeInstance( member->value, object, object->GetClass(), object );

				m_Objects.Push( object );
			}
		}
	}
#endif
	return success;
}

void Helium::Persist::ArchiveReaderBson::Resolve()
{
	ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
	info.m_Progress = 100;
	e_Status.Raise( info );

	// finish linking objects (unless we have a custom handler)
	for ( DynamicArray< Fixup >::ConstIterator itr = m_Fixups.Begin(), end = m_Fixups.End(); itr != end; ++itr )
	{
		ArchiveReader::Resolve( itr->m_Identity, itr->m_Pointer, itr->m_PointerClass );
	}

	info.m_State = ArchiveStates::Complete;
	e_Status.Raise( info );
}

void ArchiveReaderBson::DeserializeInstance( mongo::BSONElement& value, void* instance, const Structure* structure, Object* object )
{
#if 0
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Deserializing %s\n"), structure->m_Name);
#endif
	object->PreDeserialize( NULL );

	if ( value.IsObject() )
	{
		for ( rapidjson::Value::MemberIterator itr = value.MemberBegin(), end = value.MemberEnd(); itr != end; ++itr )
		{
			uint32_t fieldCrc = 0;
			if ( itr->name.IsString() )
			{
				String fieldStr;
				fieldStr = itr->name.GetString();
				fieldCrc = Helium::Crc32( fieldStr.GetData() );
			}

			const Field* field = structure->FindFieldByName( fieldCrc );
			if ( field )
			{
				object->PreDeserialize( field );

				DeserializeField( itr->value, instance, field, object );

				object->PostDeserialize( field );
			}
			else
			{
				if ( itr->name.IsString() )
				{
					HELIUM_TRACE(
						TraceLevels::Warning, 
						"ArchiveReaderBson::DeserializeInstance - Could not find field '%s' (crc=)\n", 
						itr->name.GetString(), 
						fieldCrc);
				}
				else
				{
					HELIUM_TRACE(
						TraceLevels::Warning, 
						"ArchiveReaderBson::DeserializeInstance - Could not find field (crc=)\n", 
						fieldCrc);
				}
				
			}
		}
	}

	object->PostDeserialize( NULL );
#endif
}

void ArchiveReaderBson::DeserializeField( mongo::BSONElement& value, void* instance, const Field* field, Object* object )
{
#if 0
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print(TXT("Deserializing field %s\n"), field->m_Name);
#endif
	
	if ( field->m_Count > 1 )
	{
		if ( value.IsArray() )
		{
			for ( uint32_t i=0; i<value.Size(); ++i )
			{
				if ( i < field->m_Count )
				{
					DeserializeTranslator( value[ i ], Pointer ( field, instance, object, i ), field->m_Translator, field, object );
				}
			}
		}
		else
		{
			DeserializeTranslator( value, Pointer ( field, instance, object, 0 ), field->m_Translator, field, object );
		}
	}
	else
	{
		DeserializeTranslator( value, Pointer ( field, instance, object ), field->m_Translator, field, object );
	}
#endif
}

void ArchiveReaderBson::DeserializeTranslator( mongo::BSONElement& value, Pointer pointer, Translator* translator, const Field* field, Object* object )
{
#if 0
	if ( value.IsBool() )
	{
		if ( translator->GetReflectionType() == ReflectionTypes::ScalarTranslator )
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			if ( scalar->m_Type == ScalarTypes::Boolean )
			{
				pointer.As<bool>() = value.IsTrue();
			}
		}
	}
	else if ( value.IsNumber() )
	{
		if ( translator->GetReflectionType() == ReflectionTypes::ScalarTranslator )
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			bool clamp = true;
			switch ( scalar->m_Type )
			{
			case ScalarTypes::Boolean:
			case ScalarTypes::String:
				break;
				
			case ScalarTypes::Unsigned8:
				RangeCastInteger( value.GetUint(), pointer.As<uint8_t>(), clamp );
				break;

			case ScalarTypes::Unsigned16:
				RangeCastInteger( value.GetUint(), pointer.As<uint16_t>(), clamp );
				break;

			case ScalarTypes::Unsigned32:
				RangeCastInteger( value.GetUint(), pointer.As<uint32_t>(), clamp );
				break;

			case ScalarTypes::Unsigned64:
				RangeCastInteger( value.GetUint64(), pointer.As<uint64_t>(), clamp );
				break;

			case ScalarTypes::Signed8:
				RangeCastInteger( value.GetInt(), pointer.As<int8_t>(), clamp );
				break;

			case ScalarTypes::Signed16:
				RangeCastInteger( value.GetInt(), pointer.As<int16_t>(), clamp );
				break;

			case ScalarTypes::Signed32:
				RangeCastInteger( value.GetInt(), pointer.As<int32_t>(), clamp );
				break;

			case ScalarTypes::Signed64:
				RangeCastInteger( value.GetInt64(), pointer.As<int64_t>(), clamp );
				break;

			case ScalarTypes::Float32:
				RangeCastFloat( value.GetDouble(), pointer.As<float32_t>(), clamp );
				break;

			case ScalarTypes::Float64:
				RangeCastFloat( value.GetDouble(), pointer.As<float64_t>(), clamp );
				break;
			}
		}
	}
	else if ( value.IsString() )
	{
		if ( translator->HasReflectionType( ReflectionTypes::ScalarTranslator ) )
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			if ( scalar->m_Type == ScalarTypes::String )
			{
				String str ( value.GetString() );
				scalar->Parse( str, pointer, this, m_Flags | ArchiveFlags::Notify ? true : false );
			}
		}
	}
	else if ( value.IsArray() )
	{
		if ( translator->GetReflectionType() == ReflectionTypes::SetTranslator )
		{
			SetTranslator* set = static_cast< SetTranslator* >( translator );
			Translator* itemTranslator = set->GetItemTranslator();
			uint32_t length = value.Size();
			for ( uint32_t i=0; i<length; ++i )
			{
				Variable item ( itemTranslator );
				DeserializeTranslator( value[ i ], item, itemTranslator, field, object );
				set->InsertItem( pointer, item );
			}
		}
		else if ( translator->GetReflectionType() == ReflectionTypes::SequenceTranslator )
		{
			SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );
			Translator* itemTranslator = sequence->GetItemTranslator();
			uint32_t length = value.Size();
			sequence->SetLength(pointer, length);
			for ( uint32_t i=0; i<length; ++i )
			{
				Pointer item = sequence->GetItem( pointer, i );
				DeserializeTranslator( value[ i ], item, itemTranslator, field, object );
			}
		}
	}
	else if ( value.IsObject() )
	{
		if ( translator->GetReflectionType() == ReflectionTypes::StructureTranslator )
		{
			StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
			DeserializeInstance( value, pointer.m_Address,  structure->GetStructure(), object );
		}
		else if ( translator->GetReflectionType() == ReflectionTypes::AssociationTranslator )
		{
			AssociationTranslator* assocation = static_cast< AssociationTranslator* >( translator );
			Translator* keyTranslator = assocation->GetKeyTranslator();
			Translator* valueTranslator = assocation->GetValueTranslator();
			Variable keyVariable ( keyTranslator );
			Variable valueVariable ( valueTranslator );
			for ( rapidjson::Value::MemberIterator itr = value.MemberBegin(), end = value.MemberEnd(); itr != end; ++itr )
			{
				DeserializeTranslator( itr->name, keyVariable, keyTranslator, field, object );
				DeserializeTranslator( itr->value, valueVariable, valueTranslator, field, object );
				assocation->SetItem( pointer, keyVariable, valueVariable );
			}
		}
	}
#endif
}

ObjectPtr ArchiveReaderBson::FromStream( Stream& stream, ObjectResolver* resolver, uint32_t flags )
{
	ArchiveReaderBson archive( &stream, resolver );
	archive.m_Flags = flags;
	ObjectPtr object;
	archive.Read( object );
	archive.Close();
	return object;
}

#endif