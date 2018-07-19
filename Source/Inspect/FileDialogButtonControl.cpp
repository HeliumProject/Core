#include "Precompile.h"
#include "Inspect/FileDialogButtonControl.h"

HELIUM_DEFINE_CLASS( Helium::Inspect::FileDialogButton );

using namespace Helium;
using namespace Helium::Inspect;

bool FileDialogButton::Process(const std::string& key, const std::string& value)
{
    bool wasHandled = false;

    if ( key == BUTTON_FILEDIALOG_ATTR_FILTER )
    {
        a_Filter.Set( value );
        wasHandled = true;
    }
    else if ( key == BUTTON_FILEDIALOG_ATTR_TITLE )
    {
        a_Caption.Set( value );
        wasHandled = true;
    }
    else
    {
        wasHandled = Base::Process( key, value );
    }

    return wasHandled;
}
