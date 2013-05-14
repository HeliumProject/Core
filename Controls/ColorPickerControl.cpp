#include "InspectPch.h"
#include "Inspect/Controls/ColorPickerControl.h"
#include "Inspect/Container.h"

#include <sstream>

REFLECT_DEFINE_OBJECT( Helium::Inspect::ColorPicker );

using namespace Helium;
using namespace Helium::Inspect;

ColorPicker::ColorPicker()
: a_Highlight( false )
{
    m_ContextMenu = new ContextMenu( this );
    m_ContextMenu->AddItem( TXT( "Set To Default" ), ContextMenuSignature::Delegate( this, &ColorPicker::SetToDefault ) );
}

void ColorPicker::SetDefaultAppearance( bool def ) 
{
    a_Highlight.Set( def );
}

void ColorPicker::SetToDefault( const ContextMenuEventArgs& event )
{
    event.m_Control->SetDefault();
    event.m_Control->Read();
}
