#include "Precompile.h"

#include "Math/Color.h"
#include "Reflect/TranslatorDeduction.h"

using namespace Helium;

HELIUM_DEFINE_BASE_STRUCT( Helium::Color );

void Color::PopulateMetaType( Reflect::MetaStruct& comp )
{
	comp.AddField( &Color::m_color, "m_color" );
}