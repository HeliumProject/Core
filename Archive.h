#pragma once

#include "Platform/Assert.h"

#include "Foundation/Event.h"
#include "Foundation/FilePath.h"
#include "Foundation/Log.h" 
#include "Foundation/SmartPtr.h"

#include "Reflect/Data.h"
#include "Reflect/Class.h"
#include "Reflect/Exceptions.h"

#include "Persist/API.h"

// enable verbose archive printing
//#define PERSIST_ARCHIVE_VERBOSE

namespace Helium
{
	namespace Persist
	{
		class Archive;

		namespace ArchiveFlags
		{
			enum ArchiveFlag
			{
				Status  = 1 << 0, // Display status reporting
				Sparse  = 1 << 1, // Allow sparse array populations for failed objects
			};
		}

		namespace ArchiveTypes
		{
			enum ArchiveType
			{
				Auto,
				Binary,
				Json,
			};
		}
		typedef ArchiveTypes::ArchiveType ArchiveType;

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
			tstring         m_Info;
			mutable bool    m_Abort; // flag this if you want to give up
		};
		typedef Helium::Signature< const ArchiveStatus& > ArchiveStatusSignature;

		class HELIUM_PERSIST_API Archive : public Helium::RefCountBase< Archive >
		{
		protected:
			friend class RefCountBase< Archive >;

			Archive();
			Archive( const FilePath& path );
			~Archive();

		public:
			virtual ArchiveType GetType() const = 0;
			virtual ArchiveMode GetMode() const = 0;
			inline const Helium::FilePath& GetPath() const;

			inline void Get( Reflect::ObjectPtr& object );
			inline void Put( Reflect::Object* object );

			virtual void Open() = 0;
			virtual void Close() = 0;

			ArchiveStatusSignature::Event e_Status;

		protected:
			uint32_t           m_Progress; // in bytes
			bool               m_Abort;
			FilePath           m_Path;
			Reflect::ObjectPtr m_Object;
		};
		typedef Helium::SmartPtr< Archive > ArchivePtr;

		class HELIUM_PERSIST_API ArchiveWriter : public Archive
		{
		public:
			ArchiveWriter( Reflect::ObjectIdentifier& identifier );
			ArchiveWriter( const FilePath& path, Reflect::ObjectIdentifier& identifier );

			virtual ArchiveMode GetMode() const HELIUM_OVERRIDE;
			virtual void Write() = 0;

		protected:
			Reflect::ObjectIdentifier&  m_Identifier;
		};
		typedef Helium::SmartPtr< ArchiveWriter > ArchiveWriterPtr;

		class HELIUM_PERSIST_API ArchiveReader : public Archive
		{
		public:
			ArchiveReader( Reflect::ObjectResolver& resolver );
			ArchiveReader( const FilePath& path, Reflect::ObjectResolver& resolver );

			virtual ArchiveMode GetMode() const HELIUM_OVERRIDE;
			virtual void Read() = 0;

		protected:
			Reflect::ObjectResolver&    m_Resolver;
		};
		typedef Helium::SmartPtr< ArchiveReader > ArchiveReaderPtr;

		//
		// Static API, top level functions
		//

		HELIUM_PERSIST_API ArchiveWriterPtr GetWriter( const FilePath& path, Reflect::ObjectIdentifier& identifier, ArchiveType archiveType = ArchiveTypes::Auto );
		HELIUM_PERSIST_API ArchiveReaderPtr GetReader( const FilePath& path, Reflect::ObjectResolver& resolver, ArchiveType archiveType = ArchiveTypes::Auto );

		HELIUM_PERSIST_API bool
			ToArchive( const FilePath& path, Reflect::ObjectPtr object, Reflect::ObjectIdentifier& identifier, ArchiveType archiveType = ArchiveTypes::Auto, tstring* error = NULL );

		template <class T> Helium::StrongPtr<T>
			FromArchive( const FilePath& path, Reflect::ObjectResolver& resolver, ArchiveType archiveType = ArchiveTypes::Auto );
		
		template <>
		HELIUM_PERSIST_API Helium::StrongPtr< Reflect::Object >
			FromArchive( const FilePath& path, Reflect::ObjectResolver& resolver, ArchiveType archiveType  );
	}
}

#include "Persist/Archive.inl"