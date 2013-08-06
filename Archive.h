#pragma once

#include "Platform/Assert.h"

#include "Foundation/Event.h"
#include "Foundation/FilePath.h"
#include "Foundation/Log.h" 
#include "Foundation/SmartPtr.h"

#include "Reflect/MetaClass.h"
#include "Reflect/Exceptions.h"
#include "Reflect/Object.h"
#include "Reflect/Translator.h"

#include "Persist/API.h"
#include "Persist/Exceptions.h"

// enable verbose archive printing
#define PERSIST_ARCHIVE_VERBOSE 0

namespace Helium
{
	namespace Persist
	{
		class Archive;

		namespace ArchiveFlags
		{
			enum ArchiveFlag
			{
				Notify      = 1 << 0, // Notify objects of changes
				StringCrc   = 1 << 1, // Using string CRC-32 values for meta-data instead of full strings (for brevity)
			};
		}

		namespace ArchiveTypes
		{
			enum ArchiveType
			{
				Bson,
				Json,
				MessagePack,
				Count,
				Auto = -1,
			};
		}
		typedef ArchiveTypes::ArchiveType ArchiveType;

		HELIUM_PERSIST_API extern const char* ArchiveExtensions[ ArchiveTypes::Count ];

		namespace ArchiveModes
		{
			enum ArchiveMode
			{
				Read,
				Write,
			};
		}

		typedef ArchiveModes::ArchiveMode ArchiveMode;

		namespace ArchiveStates
		{
			enum ArchiveState
			{
				Starting,
				PreProcessing,
				ArchiveStarting,
				ObjectProcessed,
				ArchiveComplete,
				PostProcessing,
				Complete,
			};
		}
		typedef ArchiveStates::ArchiveState ArchiveState;

		struct ArchiveStatus
		{
			ArchiveStatus( const Archive& archive, const ArchiveState& state )
				: m_Archive( archive )
				, m_State( state )
				, m_Progress ( 0 )
				, m_Abort ( false )
			{
			}

			const Archive&  m_Archive;
			ArchiveState    m_State;
			int             m_Progress;
			std::string     m_Info;
			mutable bool    m_Abort; // flag this if you want to give up
		};
		typedef Helium::Signature< const ArchiveStatus& > ArchiveStatusSignature;

		class HELIUM_PERSIST_API Archive : public Helium::RefCountBase< Archive >
		{
		protected:
			friend class RefCountBase< Archive >;

			Archive( uint32_t flags );
			Archive( const FilePath& path, uint32_t flags );
			~Archive();

		public:
			virtual ArchiveType GetType() const = 0;
			virtual ArchiveMode GetMode() const = 0;
			inline const Helium::FilePath& GetPath() const;

			virtual void Open() = 0;
			virtual void Close() = 0;

			ArchiveStatusSignature::Event e_Status;

		protected:
			uint32_t           m_Progress; // in bytes
			bool               m_Abort;
			const uint8_t      m_Flags;
			FilePath           m_Path;
		};
		typedef Helium::SmartPtr< Archive > ArchivePtr;

		//
		// Writer
		//

		class HELIUM_PERSIST_API ArchiveWriter : public Archive, public Reflect::ObjectIdentifier
		{
		public:
			ArchiveWriter( Reflect::ObjectIdentifier* identifier, uint32_t flags );
			ArchiveWriter( const FilePath& path, Reflect::ObjectIdentifier* identifier, uint32_t flags );

			virtual ArchiveMode GetMode() const HELIUM_OVERRIDE;
			virtual void Write( Reflect::Object* object ) = 0;
			virtual bool Identify( Reflect::Object* object, Name& identity ) HELIUM_OVERRIDE;

		protected:
			DynamicArray< Reflect::ObjectPtr > m_Objects;
			Reflect::ObjectIdentifier*         m_Identifier;
		};
		typedef Helium::SmartPtr< ArchiveWriter > ArchiveWriterPtr;

		//
		// Reader
		//

		class HELIUM_PERSIST_API ArchiveReader : public Archive, public Reflect::ObjectResolver
		{
		public:
			ArchiveReader( Reflect::ObjectResolver* resolver, uint32_t flags );
			ArchiveReader( const FilePath& path, Reflect::ObjectResolver* resolver, uint32_t flags );

			virtual ArchiveMode GetMode() const HELIUM_OVERRIDE;
			virtual void Read( Reflect::ObjectPtr& object ) = 0;
			virtual bool Resolve( const Name& identity, Reflect::ObjectPtr& pointer, const Reflect::MetaClass* pointerClass ) HELIUM_OVERRIDE;

		protected:
			struct Fixup
			{
				Fixup( const Fixup& rhs )
					: m_Identity ( rhs.m_Identity )
					, m_Pointer( rhs.m_Pointer )
					, m_PointerClass( rhs.m_PointerClass )
				{}

				Fixup( const Name& identity, Reflect::ObjectPtr& pointer, const Reflect::MetaClass* pointerClass )
					: m_Identity( identity )
					, m_Pointer( pointer )
					, m_PointerClass( pointerClass )
				{}

				Name                  m_Identity;
				Reflect::ObjectPtr&   m_Pointer;
				const Reflect::MetaClass* m_PointerClass;
			};
			DynamicArray< Fixup >              m_Fixups;
			DynamicArray< Reflect::ObjectPtr > m_Objects;
			Reflect::ObjectResolver*           m_Resolver;
		};
		typedef Helium::SmartPtr< ArchiveReader > ArchiveReaderPtr;

		//
		// Static API, top level entry points
		//

		HELIUM_PERSIST_API ArchiveWriterPtr GetWriter( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL, ArchiveType archiveType = ArchiveTypes::Auto );
		HELIUM_PERSIST_API ArchiveReaderPtr GetReader( const FilePath& path, Reflect::ObjectResolver* resolver = NULL, ArchiveType archiveType = ArchiveTypes::Auto );

		HELIUM_PERSIST_API bool ToArchive( const FilePath& path, Reflect::ObjectPtr object, Reflect::ObjectIdentifier* identifier = NULL, ArchiveType archiveType = ArchiveTypes::Auto, std::string* error = NULL );
		HELIUM_PERSIST_API bool FromArchive( const FilePath& path, Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, ArchiveType archiveType = ArchiveTypes::Auto, std::string* error = NULL );
		HELIUM_PERSIST_API Reflect::ObjectPtr FromArchive( const FilePath& path, Reflect::ObjectResolver* resolver = NULL, ArchiveType archiveType = ArchiveTypes::Auto, std::string* error = NULL );
	}
}

#include "Persist/Archive.inl"