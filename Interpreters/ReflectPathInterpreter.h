#pragma once

#include "Inspect/API.h"
#include "Inspect/Interpreters/ReflectFieldInterpreter.h"

#include "Application/FileDialog.h"

namespace Helium
{
    namespace Inspect
    {
        class HELIUM_INSPECT_API PathInterpreter : public ReflectFieldInterpreter
        {
        public:
            PathInterpreter (Container* container);

            virtual void InterpretField(const Reflect::Field* field, const std::vector<Reflect::Object*>& instances, Container* parent);

            FileDialogSignature::Delegate d_FindMissingFile;

        private:
            // callbacks
            void DataChanging( const DataChangingArgs& args );
            void Edit( const ButtonClickedArgs& args );

        protected:
            tstring m_FileFilter;
        };
    }
}