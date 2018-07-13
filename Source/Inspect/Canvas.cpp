#include "Precompile.h"
#include "Inspect/Canvas.h"

#include "Inspect/Interpreter.h"
#include "Inspect/Controls.h"

HELIUM_DEFINE_CLASS( Helium::Inspect::Canvas );

using namespace Helium;
using namespace Helium::Inspect;

Canvas::Canvas ()
	: m_DefaultSize( 20, 20 )
	, m_Border( 4 )
	, m_Pad( 2 )
{
	m_Canvas = this;
}

Canvas::~Canvas()
{
	// you *must* clear before destroying the canvas (since the canvas
	//  must still be constructed when clearing its children because
	//  Realize/Unrealize is pure virtual)
	HELIUM_ASSERT( m_Children.empty() );
}
