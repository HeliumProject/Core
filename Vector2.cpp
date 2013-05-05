#include "MathPch.h"
#include "Math/Vector2.h"
#include "Reflect/TranslatorDeduction.h"

REFLECT_DEFINE_BASE_STRUCTURE( Helium::Vector2 );

using namespace Helium;

const Vector2 Vector2::Zero;
const Vector2 Vector2::BasisX (1.0, 0.0);
const Vector2 Vector2::BasisY (0.0, 1.0);

void Vector2::PopulateStructure( Reflect::Structure& comp )
{
	comp.AddField( &Vector2::x, "x" );
	comp.AddField( &Vector2::y, "y" );
}
