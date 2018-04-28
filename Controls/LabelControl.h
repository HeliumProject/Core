#pragma once

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        const static char LABEL_ATTR_TEXT[] = "text";

        class HELIUM_INSPECT_API Label : public Control
        {
        public:
            HELIUM_DECLARE_CLASS( Label, Control );

            Label();

            virtual bool Process(const std::string& key, const std::string& value) override;

            void BindText( const std::string& text )
            {
                Bind( new StringFormatter<std::string>( new std::string( text ), true ) );
            }

            Attribute<bool> a_Ellipsize;
        };

        typedef Helium::StrongPtr<Label> LabelPtr;
    }
}