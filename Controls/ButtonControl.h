#pragma once

#include "Reflect/Object.h"

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        const static char BUTTON_ATTR_TEXT[] = TXT( "text" );
        const static char BUTTON_ATTR_ICON[] = TXT( "icon" );

        class Button;

        struct ButtonClickedArgs
        {
            ButtonClickedArgs( Button* control )
                : m_Control( control )
            {
            }

            Button* m_Control;
        };
        typedef Helium::Signature< const ButtonClickedArgs& > ButtonClickedSignature;

        class HELIUM_INSPECT_API Button : public Control
        {
        public:
            REFLECT_DECLARE_OBJECT( Button, Control );

            Button();

            ButtonClickedSignature::Event& ButtonClickedEvent()
            {
                return m_ButtonClickedEvent;
            }

            virtual bool Write() HELIUM_OVERRIDE
            {
                m_ButtonClickedEvent.Raise( ButtonClickedArgs( this ) );
                return true;
            }

        protected:
            virtual bool Process( const std::string& key, const std::string& value );

        public:
            // Label on the button
            Attribute< std::string > a_Label;

            // Icon for the button
            Attribute< std::string > a_Icon;

        protected:
            ButtonClickedSignature::Event m_ButtonClickedEvent;

        };

        typedef Helium::StrongPtr< Button > ButtonPtr;
    }
}
