#include "Precompile.h"
#include "Persist/ArchiveMessagePack.h"

#include "Foundation/Endian.h"
#include "Foundation/FileStream.h"

#include "Reflect/Object.h"
#include "Reflect/MetaStruct.h"
#include "Reflect/Registry.h"
#include "Reflect/TranslatorDeduction.h"

using namespace Helium;
using namespace Helium::Reflect;
using namespace Helium::Persist;

void ArchiveWriterMessagePack::Startup()
{
	Register( "msgpack", &AllocateWriter );
}

void ArchiveWriterMessagePack::Shutdown()
{
	Unregister( "msgpack" );
}

SmartPtr< ArchiveWriter > ArchiveWriterMessagePack::AllocateWriter( const FilePath& path, Reflect::ObjectIdentifier* identifier )
{
	return new ArchiveWriterMessagePack( path, identifier );
}

void ArchiveWriterMessagePack::WriteToStream( const ObjectPtr& object, Stream& stream, ObjectIdentifier* identifier, uint32_t flags )
{
	ArchiveWriterMessagePack archive ( &stream, identifier, flags );
	archive.Write( &object, 1 );
	archive.Close();
}

void ArchiveWriterMessagePack::WriteToStream( const ObjectPtr* objects, size_t count, Stream& stream, ObjectIdentifier* identifier, uint32_t flags )
{
	ArchiveWriterMessagePack archive ( &stream, identifier, flags );
	archive.Write( objects, count );
	archive.Close();
}

ArchiveWriterMessagePack::ArchiveWriterMessagePack( const FilePath& path, ObjectIdentifier* identifier, uint32_t flags )
	: ArchiveWriter( path, identifier, flags )
{
}

ArchiveWriterMessagePack::ArchiveWriterMessagePack( Stream *stream, ObjectIdentifier* identifier, uint32_t flags )
	: ArchiveWriter( identifier, flags )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
	m_Writer.SetStream( stream );
}

void ArchiveWriterMessagePack::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Opening file '%s'\n", m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path.Data(), FileStream::MODE_WRITE );
	m_Stream.Reset( stream );
	m_Writer.SetStream( stream );
}

void ArchiveWriterMessagePack::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close(); 
}

void ArchiveWriterMessagePack::Write( const Reflect::ObjectPtr* objects, size_t count )
{
	HELIUM_PERSIST_SCOPE_TIMER( "Reflect - MessagePack Write" );

	// notify starting
	ArchiveStatus info( *this, ArchiveStates::Starting );
	e_Status.Raise( info );

	// the master object
	m_Objects.AddArray( objects, count );

	// begin top level array of objects
	m_Writer.BeginArray();

	// objects can get changed during this iteration (in Identify), so use indices
	for ( size_t index = 0; index < m_Objects.GetSize(); ++index )
	{
		Object* object = m_Objects.GetElement( index );
		const MetaClass* objectClass = object->GetMetaClass();

		m_Writer.BeginMap( 1 );

		if ( m_Flags & ArchiveFlags::StringCrc )
		{
			uint32_t typeCrc = Crc32( objectClass->m_Name );
			m_Writer.Write( typeCrc );
		}
		else
		{
			m_Writer.Write( objectClass->m_Name );
		}

		SerializeInstance( object, objectClass, object );

		m_Writer.EndMap();

		info.m_State = ArchiveStates::ObjectProcessed;
		info.m_Progress = (int)(((float)(index) / (float)m_Objects.GetSize()) * 100.0f);
		e_Status.Raise( info );
	}

	// end top level array
	m_Writer.EndArray();

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

void ArchiveWriterMessagePack::SerializeInstance( void* instance, const MetaStruct* structure, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Serializing %s\n", structure->m_Name );
#endif

	// TODO: Declare a max depth for inheritance to save heap allocs -geoff
	DynamicArray< const MetaStruct* > bases;
	for ( const MetaStruct* current = structure; current != NULL; current = current->m_Base )
	{
		bases.Push( current );
	}

	// TODO: "Declare a max count for fields to save heap allocs -geoff
	DynamicArray< const Field* > fields;
	while ( !bases.IsEmpty() )
	{
		const MetaStruct* current = bases.Pop();
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

	m_Writer.BeginMap( static_cast< uint32_t >( fields.GetSize() ) );
	object->PreSerialize( NULL );

	DynamicArray< const Field* >::ConstIterator itr = fields.Begin();
	DynamicArray< const Field* >::ConstIterator end = fields.End();
	for ( ; itr != end; ++itr )
	{
		const Field* field = *itr;
		object->PreSerialize( field );
		SerializeField( instance, field, object );
		object->PostSerialize( field );
	}

	object->PostSerialize( NULL );
	m_Writer.EndMap();
}

void ArchiveWriterMessagePack::SerializeField( void* instance, const Field* field, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Serializing field %s\n", field->m_Name);
#endif

	if ( m_Flags & ArchiveFlags::StringCrc )
	{
		// write the crc of the field name (used to associate a field when reading)
		uint32_t fieldNameCrc = Crc32( field->m_Name );
		m_Writer.Write( fieldNameCrc );
	}
	else
	{
		// write the actual string
		m_Writer.Write( field->m_Name );
	}

	if ( field->m_Count > 1 )
	{
		m_Writer.BeginArray( field->m_Count );

		for ( uint32_t i=0; i<field->m_Count; ++i )
		{
			SerializeTranslator( Pointer ( field, instance, object, i ), field->m_Translator, field, object );
		}

		m_Writer.EndArray();
	}
	else
	{
		SerializeTranslator( Pointer ( field, instance, object ), field->m_Translator, field, object );
	}
}

void ArchiveWriterMessagePack::SerializeTranslator( Pointer pointer, Translator* translator, const Field* field, Object* object )
{
	switch ( translator->GetMetaId() )
	{
	case MetaIds::ScalarTranslator:
	case MetaIds::SimpleTranslator:
	case MetaIds::EnumerationTranslator:
	case MetaIds::PointerTranslator:
	case MetaIds::TypeTranslator:
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			switch ( scalar->m_Type )
			{
			case ScalarTypes::Boolean:
				m_Writer.Write( pointer.As<bool>() );
				break;

			case ScalarTypes::Unsigned8:
				m_Writer.Write( pointer.As<uint8_t>() );
				break;

			case ScalarTypes::Unsigned16:
				m_Writer.Write( pointer.As<uint16_t>() );
				break;

			case ScalarTypes::Unsigned32:
				m_Writer.Write( pointer.As<uint32_t>() );
				break;

			case ScalarTypes::Unsigned64:
				m_Writer.Write( pointer.As<uint64_t>() );
				break;

			case ScalarTypes::Signed8:
				m_Writer.Write( pointer.As<int8_t>() );
				break;

			case ScalarTypes::Signed16:
				m_Writer.Write( pointer.As<int16_t>() );
				break;

			case ScalarTypes::Signed32:
				m_Writer.Write( pointer.As<int32_t>() );
				break;

			case ScalarTypes::Signed64:
				m_Writer.Write( pointer.As<int64_t>() );
				break;

			case ScalarTypes::Float32:
				m_Writer.Write( pointer.As<float32_t>() );
				break;

			case ScalarTypes::Float64:
				m_Writer.Write( pointer.As<float64_t>() );
				break;

			case ScalarTypes::String:
				String str;
				scalar->Print( pointer, str, this );
				m_Writer.Write( str.GetData() );
				break;
			}
			break;
		}

	case MetaIds::StructureTranslator:
		{
			StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
			SerializeInstance( pointer.m_Address, structure->GetMetaStruct(), object );
			break;
		}

	case MetaIds::SetTranslator:
		{
			SetTranslator* set = static_cast< SetTranslator* >( translator );

			Translator* itemTranslator = set->GetItemTranslator();
			DynamicArray< Pointer > items;
			set->GetItems( pointer, items );

			uint32_t length = static_cast< uint32_t >( items.GetSize() );
			m_Writer.BeginArray( length );

			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr )
			{
				SerializeTranslator( *itr, itemTranslator, field, object );
			}

			m_Writer.EndArray();

			break;
		}

	case MetaIds::SequenceTranslator:
		{
			SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );

			Translator* itemTranslator = sequence->GetItemTranslator();
			DynamicArray< Pointer > items;
			sequence->GetItems( pointer, items );

			uint32_t length = static_cast< uint32_t >( items.GetSize() );
			m_Writer.BeginArray( length );

			for ( DynamicArray< Pointer >::Iterator itr = items.Begin(), end = items.End(); itr != end; ++itr )
			{
				SerializeTranslator( *itr, itemTranslator, field, object );
			}

			m_Writer.EndArray();

			break;
		}

	case MetaIds::AssociationTranslator:
		{
			AssociationTranslator* association = static_cast< AssociationTranslator* >( translator );

			Translator* keyTranslator = association->GetKeyTranslator();
			Translator* valueTranslator = association->GetValueTranslator();
			DynamicArray< Pointer > keys, values;
			association->GetItems( pointer, keys, values );

			uint32_t length = static_cast< uint32_t >( keys.GetSize() );
			m_Writer.BeginMap( length );

			for ( DynamicArray< Pointer >::Iterator keyItr = keys.Begin(), valueItr = values.Begin(), keyEnd = keys.End(), valueEnd = values.End();
				keyItr != keyEnd && valueItr != valueEnd;
				++keyItr, ++valueItr )
			{
				SerializeTranslator( *keyItr, keyTranslator, field, object );
				SerializeTranslator( *valueItr, valueTranslator, field, object );
			}

			m_Writer.EndMap();

			break;
		}

	default:
		// Unhandled reflection type in ArchiveWriterMessagePack::SerializeTranslator
		HELIUM_BREAK();
	}
}

void ArchiveReaderMessagePack::Startup()
{
	Register( "msgpack", &AllocateReader );
}

void ArchiveReaderMessagePack::Shutdown()
{
	Unregister( "msgpack" );
}

SmartPtr< ArchiveReader > ArchiveReaderMessagePack::AllocateReader( const FilePath& path, Reflect::ObjectResolver* resolver )
{
	return new ArchiveReaderMessagePack( path, resolver );
}

void ArchiveReaderMessagePack::ReadFromStream( Stream& stream, ObjectPtr& object, ObjectResolver* resolver, uint32_t flags )
{
	DynamicArray< ObjectPtr > objects;
	ReadFromStream( stream, objects, resolver, flags );
	if ( !objects.IsEmpty() )
	{
		object = objects.GetFirst();
	}
}

void ArchiveReaderMessagePack::ReadFromStream( Stream& stream, DynamicArray< ObjectPtr >& objects, ObjectResolver* resolver, uint32_t flags )
{
	ArchiveReaderMessagePack archive( &stream, resolver, flags );
	archive.Read( objects );
	archive.Close();
}

ArchiveReaderMessagePack::ArchiveReaderMessagePack( const FilePath& path, ObjectResolver* resolver, uint32_t flags )
	: ArchiveReader( path, resolver, flags )
	, m_Stream( NULL )
	, m_Size( 0 )
{
}

ArchiveReaderMessagePack::ArchiveReaderMessagePack( Stream *stream, ObjectResolver* resolver, uint32_t flags )
	: ArchiveReader( resolver, flags )
	, m_Stream( NULL )
	, m_Size( 0 )
{
	m_Stream.Reset( stream );
	m_Stream.Orphan( true );
	m_Reader.SetStream( stream );
}

void ArchiveReaderMessagePack::Open()
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Opening file '%s'\n", m_Path.c_str());
#endif

	FileStream* stream = new FileStream();
	stream->Open( m_Path.Data(), FileStream::MODE_READ );
	m_Stream.Reset( stream );
	m_Reader.SetStream( stream );
}

void ArchiveReaderMessagePack::Close()
{
	HELIUM_ASSERT( m_Stream );
	m_Stream->Close(); 
}

void ArchiveReaderMessagePack::Read( DynamicArray< ObjectPtr >& objects )
{
	HELIUM_PERSIST_SCOPE_TIMER( "Reflect - MessagePack Read" );

	Start();

	m_Objects = objects;

	if ( HELIUM_VERIFY( m_Reader.IsArray() ) )
	{
		uint32_t length = m_Reader.ReadArrayLength();

		m_Objects.Resize( length );

		m_Reader.BeginArray( length );

		for ( uint32_t i=0; i<length; i++ )
		{
			ObjectPtr& object( m_Objects[ i ] );
			ReadNext( object, i );

			ArchiveStatus info( *this, ArchiveStates::ObjectProcessed );
			info.m_Progress = (int)(((float)(m_Stream->Tell()) / (float)m_Size) * 100.0f);
			e_Status.Raise( info );
			m_Abort |= info.m_Abort;
			if ( m_Abort )
			{
				break;
			}
		}

		m_Reader.EndArray();
	}

	Resolve();

	objects = m_Objects;
}

void ArchiveReaderMessagePack::Start()
{
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
		throw Persist::StreamException( "Input stream is empty (%s)", m_Path.Data() );
	}

	// parse the first byte of the stream
	m_Reader.Advance();
}

bool ArchiveReaderMessagePack::ReadNext( ObjectPtr& object, size_t index )
{
	if ( m_Stream->Tell() == this->m_Size )
	{
		return false;
	}

	if ( HELIUM_VERIFY( m_Reader.IsMap() ) )
	{
		uint32_t length = m_Reader.ReadMapLength();
		HELIUM_ASSERT( length == 1 );
		m_Reader.BeginMap( length );

		uint32_t objectClassCrc = 0;
		if ( m_Reader.IsNumber() )
		{
			m_Reader.Read( objectClassCrc, NULL );
		}
		else
		{
			String typeStr;
			m_Reader.Read( typeStr );
			objectClassCrc = Helium::Crc32( typeStr.GetData() );
		}

		const MetaClass* objectClass = NULL;
		if ( objectClassCrc != 0 )
		{
			objectClass = Registry::GetInstance()->GetMetaClass( objectClassCrc );
		}

		if ( !object && HELIUM_VERIFY( objectClass ) )
		{
			object = AllocateObject( objectClass, index );
		}

		if ( object.ReferencesObject() )
		{
			DeserializeInstance( object, object->GetMetaClass(), object );
		}
		else // object.ReferencesObject()
		{
			m_Reader.Skip();
		}

		m_Reader.EndMap();
	}

	return true;
}

void ArchiveReaderMessagePack::DeserializeInstance( void* instance, const MetaStruct* structure, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Deserializing %s\n", structure->m_Name);
#endif

	object->PreDeserialize( NULL );

	if ( HELIUM_VERIFY( m_Reader.IsMap() ) )
	{
		uint32_t length = m_Reader.ReadMapLength();
		m_Reader.BeginMap( length );

		for (uint32_t i=0; i<length; i++)
		{
			uint32_t fieldCrc = 0;
			if ( m_Reader.IsNumber() )
			{
				m_Reader.Read( fieldCrc, NULL );
			}
			else
			{
				String fieldStr;
				m_Reader.Read( fieldStr );
				fieldCrc = Helium::Crc32( fieldStr.GetData() );
			}

			const Field* field = structure->FindFieldByName( fieldCrc );
			if ( field )
			{
				object->PreDeserialize( field );

				DeserializeField( instance, field, object );

				object->PostDeserialize( field );
			}
			else
			{
				m_Reader.Skip();
			}
		}

		m_Reader.EndMap();
	}
	else // IsMap()
	{
		m_Reader.Skip();
	}

	object->PostDeserialize( NULL );
}

void ArchiveReaderMessagePack::DeserializeField( void* instance, const Field* field, Object* object )
{
#if PERSIST_ARCHIVE_VERBOSE
	Log::Print("Deserializing field %s\n", field->m_Name);
#endif
	
	if ( field->m_Count > 1 )
	{
		if ( m_Reader.IsArray() )
		{
			uint32_t length = m_Reader.ReadArrayLength();
			m_Reader.BeginArray( length );
			for ( uint32_t i=0; i<length; ++i )
			{
				if ( i < field->m_Count )
				{
					DeserializeTranslator( Pointer ( field, instance, object, i ), field->m_Translator, field, object );
				}
				else
				{
					m_Reader.Skip();
				}
			}
			m_Reader.EndArray();
		}
		else
		{
			DeserializeTranslator( Pointer ( field, instance, object, 0 ), field->m_Translator, field, object );
		}
	}
	else
	{
		DeserializeTranslator( Pointer ( field, instance, object ), field->m_Translator, field, object );
	}
}

void ArchiveReaderMessagePack::DeserializeTranslator( Pointer pointer, Translator* translator, const Field* field, Object* object )
{
	if ( m_Reader.IsBoolean() )
	{
		if ( translator->IsA(MetaIds::ScalarTranslator) )
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			if ( scalar->m_Type == ScalarTypes::Boolean )
			{
				m_Reader.Read( pointer.As<bool>(), NULL );
			}
			else
			{
				m_Reader.Skip(); // no implicit conversion, discard data
			}
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else if ( m_Reader.IsNumber() )
	{
		if ( translator->IsA(MetaIds::ScalarTranslator) )
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			bool clamp = true;
			switch ( scalar->m_Type )
			{
			case ScalarTypes::Unsigned8:
				m_Reader.ReadNumber( pointer.As<uint8_t>(), clamp, NULL );
				break;

			case ScalarTypes::Unsigned16:
				m_Reader.ReadNumber( pointer.As<uint16_t>(), clamp, NULL );
				break;

			case ScalarTypes::Unsigned32:
				m_Reader.ReadNumber( pointer.As<uint32_t>(), clamp, NULL );
				break;

			case ScalarTypes::Unsigned64:
				m_Reader.ReadNumber( pointer.As<uint64_t>(), clamp, NULL );
				break;

			case ScalarTypes::Signed8:
				m_Reader.ReadNumber( pointer.As<int8_t>(), clamp, NULL );
				break;

			case ScalarTypes::Signed16:
				m_Reader.ReadNumber( pointer.As<int16_t>(), clamp, NULL );
				break;

			case ScalarTypes::Signed32:
				m_Reader.ReadNumber( pointer.As<int32_t>(), clamp, NULL );
				break;

			case ScalarTypes::Signed64:
				m_Reader.ReadNumber( pointer.As<int64_t>(), clamp, NULL );
				break;

			case ScalarTypes::Float32:
				m_Reader.ReadNumber( pointer.As<float32_t>(), clamp, NULL );
				break;

			case ScalarTypes::Float64:
				m_Reader.ReadNumber( pointer.As<float64_t>(), clamp, NULL );
				break;

			default:
				m_Reader.Skip(); // no implicit conversion, discard data
				break;
			}
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else if ( m_Reader.IsRaw() )
	{
		if ( translator->IsA( MetaIds::ScalarTranslator ) )
		{
			ScalarTranslator* scalar = static_cast< ScalarTranslator* >( translator );
			if ( scalar->m_Type == ScalarTypes::String )
			{
				String str;
				m_Reader.Read( str );
				scalar->Parse( str, pointer, this, m_Flags | ArchiveFlags::Notify ? true : false );
			}
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else if ( m_Reader.IsArray() )
	{
		if ( translator->GetMetaId() == MetaIds::SetTranslator )
		{
			SetTranslator* set = static_cast< SetTranslator* >( translator );
			Translator* itemTranslator = set->GetItemTranslator();
			uint32_t length = m_Reader.ReadArrayLength();
			m_Reader.BeginArray( length );
			for ( uint32_t i=0; i<length; ++i )
			{
				Variable item ( itemTranslator );
				DeserializeTranslator( item, itemTranslator, field, object );
				set->InsertItem( pointer, item );
			}
			m_Reader.EndArray();
		}
		else if ( translator->GetMetaId() == MetaIds::SequenceTranslator )
		{
			SequenceTranslator* sequence = static_cast< SequenceTranslator* >( translator );
			Translator* itemTranslator = sequence->GetItemTranslator();
			uint32_t length = m_Reader.ReadArrayLength();
			sequence->SetLength(pointer, length);
			m_Reader.BeginArray( length );
			for ( uint32_t i=0; i<length; ++i )
			{
				Pointer item = sequence->GetItem( pointer, i );
				DeserializeTranslator( item, itemTranslator, field, object );
			}
			m_Reader.EndArray();
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else if ( m_Reader.IsMap() )
	{
		if ( translator->GetMetaId() == MetaIds::StructureTranslator )
		{
			StructureTranslator* structure = static_cast< StructureTranslator* >( translator );
			DeserializeInstance( pointer.m_Address,  structure->GetMetaStruct(), object );
		}
		else if ( translator->GetMetaId() == MetaIds::AssociationTranslator )
		{
			AssociationTranslator* assocation = static_cast< AssociationTranslator* >( translator );
			Translator* keyTranslator = assocation->GetKeyTranslator();
			Translator* valueTranslator = assocation->GetValueTranslator();
			uint32_t length = m_Reader.ReadMapLength();
			m_Reader.BeginMap( length );
			for ( uint32_t i=0; i<length; ++i )
			{
				Variable key ( keyTranslator );
				Variable value ( valueTranslator );
				DeserializeTranslator( key, keyTranslator, field, object );
				DeserializeTranslator( value, valueTranslator, field, object );
				assocation->SetItem( pointer, key, value );
			}
			m_Reader.EndMap();
		}
		else
		{
			m_Reader.Skip(); // no implicit conversion, discard data
		}
	}
	else
	{
		m_Reader.Skip(); // no implicit conversion, discard data
	}
}
