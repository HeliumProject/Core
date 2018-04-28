#include "Precompile.h"
#include "Inspect/Controls/CheckBoxControl.h"
#include "Inspect/Container.h"

HELIUM_DEFINE_CLASS( Helium::Inspect::CheckBox );

using namespace Helium;
using namespace Helium::Inspect;

CheckBox::CheckBox()
: a_Highlight( false )
{
    a_IsFixedWidth.Set( true );

    m_ContextMenu = new ContextMenu( this );
    m_ContextMenu->AddItem( "Set To Default", ContextMenuSignature::Delegate( this, &CheckBox::SetToDefault ) );
}

void CheckBox::SetDefaultAppearance( bool def ) 
{
    a_Highlight.Set( def );
}

void CheckBox::SetToDefault( const ContextMenuEventArgs& event )
{
    event.m_Control->SetDefault();
    event.m_Control->Read();
}
