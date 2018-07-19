#include "Precompile.h"
#include "Inspect/SliderControl.h"
#include "Inspect/Canvas.h"

HELIUM_DEFINE_CLASS( Helium::Inspect::Slider );

using namespace Helium;
using namespace Helium::Inspect;

Slider::Slider()
: a_Min( 0.0f )
, a_Max( 100.0f )
, a_AutoAdjustMinMax( true )
{
    m_ContextMenu = new ContextMenu (this);
    m_ContextMenu->AddItem( "Set To Default", ContextMenuSignature::Delegate(this, &Slider::SetToDefault));
}

bool Slider::Process( const std::string& key, const std::string& value )
{
    if (Base::Process(key, value))
    {
        return true;
    }

    if (key == SLIDER_ATTR_MIN)
    {
		float min = 0.f;
		std::stringstream str ( value );
		str >> min;
        a_Min.Set( min );
        return true;
    }
    else if (key == SLIDER_ATTR_MAX)
    {
		float max = 0.f;
		std::stringstream str ( value );
		str >> max;
        a_Max.Set( max );
        return true;
    }

    return false;
}

void Slider::SetToDefault(const ContextMenuEventArgs& event)
{
    event.m_Control->SetDefault();
    event.m_Control->Read();
}
