#include "Precompile.h"
#include "Math/Point.h"

#include "Reflect/TranslatorDeduction.h"

using namespace Helium;

const Point Point::Zero;

HELIUM_DEFINE_BASE_STRUCT( Helium::Point );

void Helium::Point::PopulateMetaType( Reflect::MetaStruct& comp )
{
	comp.AddField( &Point::x, "x" );
	comp.AddField( &Point::y, "y" );
}