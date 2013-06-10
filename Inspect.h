#pragma once

// Includes
#include "Inspect/API.h"
#include "Application/Startup.h"
#include "Foundation/Event.h"

namespace Helium
{
    namespace Inspect
    {
        HELIUM_INSPECT_API void Initialize();
        HELIUM_INSPECT_API void Cleanup();

        struct EditFilePathArgs
        {
            std::string m_File;

            EditFilePathArgs( const std::string& file )
                : m_File( file )
            {

            }
        };
        typedef Helium::Signature< const EditFilePathArgs&> EditFilePathSignature;

        extern HELIUM_INSPECT_API EditFilePathSignature::Event g_EditFilePath;
    }
}