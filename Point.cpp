#include "MathPch.h"
#include "Math/Point.h"

#include "Reflect/TranslatorDeduction.h"

using namespace Helium;

const Point Point::Zero;

REFLECT_DEFINE_BASE_STRUCTURE( Helium::Point );

void Helium::Point::PopulateStructure( Reflect::Structure& comp )
{
	comp.AddField( &Point::x, "x" );
	comp.AddField( &Point::y, "y" );
}