#pragma once

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        class List;

        class ClientDataFilter : public ClientData
        {
        public:
            REFLECT_DECLARE_ABSTRACT( ClientDataFilter, ClientData );

            ClientDataFilter( List* list, const Reflect::MetaClass* instanceClass, const std::string& filter )
                : m_List( list )
                , m_InstanceClass( instanceClass )
                , m_FileFilter( filter )
            {

            }

            List*                   m_List;
            const Reflect::MetaClass*   m_InstanceClass;
            std::string                 m_FileFilter;
        };
    }

    namespace InspectReflect
    {
        HELIUM_INSPECT_API void Initialize();
        HELIUM_INSPECT_API void Cleanup();
    }
}