#pragma once

#include <string>

#include "Platform/Types.h"

#include "Application/API.h"
#include "Foundation/SmartPtr.h"

namespace Helium
{
    namespace RCS
    {
        static const uint64_t InvalidChangesetId = 0xFFFFFFFFFFFFFFFF;
        static const uint64_t DefaultChangesetId = 0x0;

        HELIUM_APPLICATION_API std::string GetChangesetIdAsString( const uint64_t changesetId );

        namespace FileStates
        {
            enum FileState
            {
                Unknown           = 0,
                ExistsInDepot     = 1 << 0,
                HeadDeleted       = 1 << 1,
                LocalDeleted      = 1 << 2, 
                CheckedOut        = 1 << 3,
                CheckedOutByMe    = 1 << 4,
            };
        }
        typedef FileStates::FileState FileState;

        namespace FileFlags
        {
            enum FileFlag
            {
                Unknown   = 0,
                Locking   = 1 << 1,
                HeadOnly  = 1 << 2,
            };
        }
        typedef FileFlags::FileFlag FileFlag;

        namespace Operations
        {
            enum Operation
            {
                Unknown,
                None,
                Add,
                Edit,
                Delete,
                Branch,
                Integrate,
            };
        }
        typedef Operations::Operation Operation;

        namespace FileTypes
        {
            enum FileType
            {
                Unknown,
                Text,
                Binary,
                Directory,
            };
        }
        typedef FileTypes::FileType FileType;

        enum UserSelection
        {
            Yes,
            No,
            Cancel
        };

        namespace GetInfoFlags
        {
            enum GetInfoFlag
            {
                None                  = 0,
                GetHistory            = 1 << 0,
                GetIntegrationHistory = 1 << 1,

                Default = None
            };
        }
        typedef GetInfoFlags::GetInfoFlag GetInfoFlag;

        namespace OpenFlags
        {
            enum OpenFlag
            {
                None           = 0,
                Exclusive      = 1 << 0,
                UnchangedOnly  = 1 << 1,
                AllowOutOfDate = 1 << 2, 

                Default       = None
            };
        }
        typedef OpenFlags::OpenFlag OpenFlag;
    }
}