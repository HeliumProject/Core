#include "Precompile.h"
#include "Math/Vector2.h"
#include "Reflect/TranslatorDeduction.h"

HELIUM_DEFINE_BASE_STRUCT( Helium::Vector2 );

using namespace Helium;

const Vector2 Vector2::Zero;
const Vector2 Vector2::BasisX (1.0, 0.0);
const Vector2 Vector2::BasisY (0.0, 1.0);

void Vector2::PopulateMetaType( Reflect::MetaStruct& comp )
{
	comp.AddField( &Vector2::x, "x" );
	comp.AddField( &Vector2::y, "y" );
}
