#include "InspectPch.h"
#include "Inspect/Controls/ButtonControl.h"
#include "Inspect/Container.h"
#include "Inspect/Canvas.h"

REFLECT_DEFINE_CLASS( Helium::Inspect::Button );

using namespace Helium;
using namespace Helium::Inspect;

Button::Button()
{
}

bool Button::Process( const std::string& key, const std::string& value )
{
    if ( Base::Process(key, value) )
    {
        return true;
    }

    if ( key == BUTTON_ATTR_TEXT )
    {
        a_Label.Set( value );
        return true;
    }

    if ( key == BUTTON_ATTR_ICON )
    {
        a_Icon.Set( value );
        return true;
    }

    return false;
}
