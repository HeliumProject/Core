#pragma once

#include "Platform/Assert.h"

#include "Foundation/Event.h"
#include "Foundation/FilePath.h"
#include "Foundation/Log.h" 
#include "Foundation/SmartPtr.h"

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
				Base
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

		//
		// Status reporting
		//

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
				Publishing,
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

			// flag this if you want to give up
			mutable bool    m_Abort;
		};
		typedef Helium::Signature< const ArchiveStatus& > ArchiveStatusSignature;

		//
		// Archive base class
		//

		class HELIUM_PERSIST_API Archive : public Helium::RefCountBase< Archive >
		{
		protected:
			friend class RefCountBase< Archive >;

			Archive();
			Archive( const FilePath& path );
			~Archive();

		public:
			const Helium::FilePath& GetPath() const
			{
				return m_Path;
			}

			ArchiveMode GetMode() const
			{
				return m_Mode;
			}

			virtual ArchiveType GetType() const
			{
				return ArchiveTypes::Base;
			}

			//
			// Virutal functionality, meant to be overridden by Binary/XML/etc. archives
			//

			// File Open/Close
			virtual void Open( bool write = false ) = 0;
			virtual void Close() = 0;

			// Begins parsing the InputStream
			virtual void Read() = 0;

			// Write to the OutputStream
			virtual void Write() = 0;

			//
			// Event API
			//

			ArchiveStatusSignature::Event e_Status;

			//
			// Get objects from the file
			//

			void Put( Reflect::Object* object );
			Reflect::ObjectPtr Get( const Reflect::Class* searchClass = NULL );
			void Get( Reflect::ObjectPtr& object );

			// Get a single object of the specified type in the archive
			template <class T>
			Helium::StrongPtr<T> Get()
			{
				Reflect::ObjectPtr found = Get( Reflect::GetClass<T>() );

				if (found.ReferencesObject())
				{
					return SafeCast<T>( found );
				}
				else
				{
					return NULL;
				}
			}

		protected:
			// The number of bytes Parsed so far
			unsigned m_Progress;

			// The abort status
			bool m_Abort;

			// The file we are working with
			FilePath m_Path;

			// The mode
			ArchiveMode m_Mode;

			// The array of objects that we've found
			Reflect::ObjectPtr m_Object;
		};

		typedef Helium::SmartPtr< Archive > ArchivePtr;

		// Get parser for a file
		HELIUM_PERSIST_API ArchivePtr GetArchive( const FilePath& path, ArchiveType archiveType = ArchiveTypes::Auto );

		// Write an object to a file
		HELIUM_PERSIST_API bool ToArchive( const FilePath& path, Reflect::ObjectPtr object, ArchiveType archiveType = ArchiveTypes::Auto, tstring* error = NULL );

		// Parse an object from a file
		template <class T>
		Helium::StrongPtr<T> FromArchive( const FilePath& path, ArchiveType archiveType = ArchiveTypes::Auto )
		{
			ArchivePtr archive = GetArchive( path, archiveType );

			if ( archive.ReferencesObject() )
			{
				return archive->Get< T >();
			}

			return NULL;
		}
	}
}