#include "Precompile.h"
#include "Inspect/LabelControl.h"
#include "Inspect/Container.h"
#include "Inspect/DataBinding.h"

HELIUM_DEFINE_CLASS( Helium::Inspect::Label );

using namespace Helium;
using namespace Helium::Inspect;

Label::Label()
: a_Ellipsize( true )
{
    a_ProportionalWidth.Set( 1.f/3.f );
}

bool Label::Process(const std::string& key, const std::string& value)
{
    bool handled = false;

    if ( Base::Process(key, value) )
    {
        return true;
    }

    if ( key == LABEL_ATTR_TEXT )
    {
        Bind( new StringFormatter<std::string>( new std::string( value ), true ) );
        return true;
    }

    return false;
}
