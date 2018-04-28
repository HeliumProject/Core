#pragma once

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        const static char TEXTBOX_ATTR_REQUIRED[] = "required";
        const static char TEXTBOX_ATTR_JUSTIFY[] = "justify";
        const static char TEXTBOX_ATTR_JUSTIFY_LEFT[] = "left";
        const static char TEXTBOX_ATTR_JUSTIFY_RIGHT[] = "right";

        namespace Justifications
        {
            enum Justification
            {
                Left,
                Right,
            };
        };
        typedef Justifications::Justification Justification;

        class HELIUM_INSPECT_API Value : public Control
        {
        public:
            HELIUM_DECLARE_CLASS( Value, Control );

            Value();

        protected:
            virtual bool Process(const std::string& key, const std::string& value) override;
            virtual void SetDefaultAppearance(bool def) override;
            void SetToDefault(const ContextMenuEventArgs& event);

        public:
            Attribute< Justification >  a_Justification;
            Attribute< bool >           a_Highlight;
        };

        typedef Helium::StrongPtr<Value> ValuePtr;
    }
}