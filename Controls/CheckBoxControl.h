#pragma once

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        class HELIUM_INSPECT_API CheckBox : public Control
        {
        public:
            HELIUM_DECLARE_CLASS( CheckBox, Control );

            CheckBox();

        protected:
            virtual void SetDefaultAppearance( bool def ) override;
            void SetToDefault( const ContextMenuEventArgs& event );

        public:
            Attribute< bool >   a_Highlight;
        };

        typedef Helium::StrongPtr< CheckBox > CheckBoxPtr;
    }
}