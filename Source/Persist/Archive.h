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
		HELIUM_PERSIST_API void Startup();
		HELIUM_PERSIST_API void Shutdown();

		class Archive;

		namespace ArchiveFlags
		{
			enum ArchiveFlag
			{
				Notify      = 1 << 0, // Notify objects of changes
				StringCrc   = 1 << 1, // Using string CRC-32 values for meta-data instead of full strings (for brevity)
			};
		}

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

		//
		// Base class for Readers and Writers
		//

		class HELIUM_PERSIST_API Archive : public Helium::RefCountBase< Archive >
		{
		protected:
			friend class RefCountBase< Archive >;

			Archive( uint32_t flags );
			Archive( const FilePath& path, uint32_t flags );
			~Archive();

		public:
			inline const Helium::FilePath& GetPath() const;

			virtual ArchiveMode GetMode() const = 0;
			virtual void        Open() = 0;
			virtual void        Close() = 0;

			ArchiveStatusSignature::Event e_Status;

		protected:
			uint32_t           m_Progress; // in bytes
			bool               m_Abort;
			const uint8_t      m_Flags;
			FilePath           m_Path;
		};

		//
		// Writer
		//

		class HELIUM_PERSIST_API ArchiveWriter : public Archive, public Reflect::ObjectIdentifier
		{
		protected:
			typedef SmartPtr< ArchiveWriter > (*Allocate)( const FilePath& path, Reflect::ObjectIdentifier* identifier );
			
			struct Registration
			{
				char m_Extension[16];
				Allocate m_Allocator;
			};

			static void Register( const char* extension, Allocate allocator );
			static void Unregister( const char* extension );
			static DynamicArray<Registration> s_Writers;

		public:
			static SmartPtr< ArchiveWriter > GetWriter( const FilePath& path, Reflect::ObjectIdentifier* identifier = NULL );
			static bool                      WriteToFile( const FilePath& path, const Reflect::ObjectPtr& object, Reflect::ObjectIdentifier* identifier = NULL, std::string* error = NULL );
			static bool                      WriteToFile( const FilePath& path, const Reflect::ObjectPtr* objects, size_t count, Reflect::ObjectIdentifier* identifier = NULL, std::string* error = NULL );

			ArchiveWriter( Reflect::ObjectIdentifier* identifier, uint32_t flags );
			ArchiveWriter( const FilePath& path, Reflect::ObjectIdentifier* identifier, uint32_t flags );

			virtual ArchiveMode GetMode() const override;

		protected:
			virtual void Write( const Reflect::ObjectPtr* objects, size_t count ) = 0;
			virtual bool Identify( const Reflect::ObjectPtr& object, Name* identity ) override;

			DynamicArray< Reflect::ObjectPtr > m_Objects;
			Reflect::ObjectIdentifier*         m_Identifier;
		};

		//
		// Reader
		//

		class HELIUM_PERSIST_API ArchiveReader : public Archive, public Reflect::ObjectResolver
		{
		protected:
			typedef SmartPtr< ArchiveReader > (*Allocate)( const FilePath& path, Reflect::ObjectResolver* resolver );

			struct Registration
			{
				char m_Extension[16];
				Allocate m_Allocator;
			};

			static void Register( const char* extension, Allocate allocator );
			static void Unregister( const char* extension );
			static DynamicArray<Registration> s_Readers;

		public:
			static SmartPtr< ArchiveReader > GetReader( const FilePath& path, Reflect::ObjectResolver* resolver = NULL );
			static bool                      ReadFromFile( const FilePath& path, Reflect::ObjectPtr& object, Reflect::ObjectResolver* resolver = NULL, std::string* error = NULL );
			static Reflect::ObjectPtr        ReadFromFile( const FilePath& path, Reflect::ObjectResolver* resolver = NULL, std::string* error = NULL );
			static bool                      ReadFromFile( const FilePath& path, DynamicArray< Reflect::ObjectPtr >& objects, Reflect::ObjectResolver* resolver = NULL, std::string* error = NULL );

			ArchiveReader( Reflect::ObjectResolver* resolver, uint32_t flags );
			ArchiveReader( const FilePath& path, Reflect::ObjectResolver* resolver, uint32_t flags );

			virtual ArchiveMode GetMode() const override;

		protected:
			virtual void       Read( DynamicArray< Reflect::ObjectPtr >& objects ) = 0;
			Reflect::ObjectPtr AllocateObject( const Reflect::MetaClass* type, size_t index );
			bool               Resolve( const Name& identity, Reflect::ObjectPtr& pointer, const Reflect::MetaClass* pointerClass ) override;
			void               Resolve();

			struct Fixup
			{
				Fixup( const Fixup& rhs )
					: m_Index ( rhs.m_Index )
					, m_PointerClass( rhs.m_PointerClass )
				{}

				Fixup( size_t index, const Reflect::MetaClass* pointerClass )
					: m_Index( index )
					, m_PointerClass( pointerClass )
				{}

				size_t                    m_Index;
				const Reflect::MetaClass* m_PointerClass;
			};

			std::vector< RefCountProxy< Reflect::Object >* >  m_Proxies;
			DynamicArray< Fixup >                             m_Fixups;
			DynamicArray< Reflect::ObjectPtr >                m_Objects;
			Reflect::ObjectResolver*                          m_Resolver;
		};
	}
}

#include "Persist/Archive.inl"