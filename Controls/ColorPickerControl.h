#pragma once

#include "MathSimd/Color.h"
#include "Reflect/MetaClass.h"

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        class HELIUM_INSPECT_API ColorPicker : public Control
        {
        public:
            HELIUM_DECLARE_CLASS( ColorPicker, Control );

            ColorPicker();

        protected:
            virtual void SetDefaultAppearance( bool def ) HELIUM_OVERRIDE;
            void SetToDefault( const ContextMenuEventArgs& event );

        public:
            Attribute< bool >       a_Highlight;
            Attribute< bool >       a_Alpha;
            Attribute< Color >      a_Color;
        };

        typedef Helium::StrongPtr< ColorPicker > ColorPickerPtr;
    }
}
