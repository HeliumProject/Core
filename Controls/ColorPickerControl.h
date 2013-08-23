#pragma once

#include "Math/Color3.h"
#include "Math/Color4.h"
#include "Math/HDRColor3.h"
#include "Math/HDRColor4.h"
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
            Attribute< Color3 >     a_Color3;
            Attribute< Color4 >     a_Color4;
        };

        typedef Helium::StrongPtr< ColorPicker > ColorPickerPtr;
    }
}
