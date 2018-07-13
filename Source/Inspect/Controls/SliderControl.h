#pragma once

#include "Inspect/API.h"
#include "Inspect/Control.h"

namespace Helium
{
    namespace Inspect
    {
        const static char SLIDER_ATTR_MIN[] = "min";
        const static char SLIDER_ATTR_MAX[] = "max";

        class HELIUM_INSPECT_API Slider : public Control
        {
        public:
            HELIUM_DECLARE_CLASS( Slider, Control );

            Slider();

        protected:
            virtual bool Process( const std::string& key, const std::string& value ) override;
            
            void SetToDefault(const ContextMenuEventArgs& event);

        public:
            Attribute< float >  a_Min;
            Attribute< float >  a_Max;
            Attribute< bool >   a_AutoAdjustMinMax;
        };

        typedef Helium::StrongPtr<Slider> SliderPtr;
    }
}
