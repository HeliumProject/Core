#include "Precompile.h"
#include "Math/Vector4.h"
#include "Reflect/TranslatorDeduction.h"

HELIUM_DEFINE_BASE_STRUCT( Helium::Vector4 );

using namespace Helium;

const Vector4 Vector4::Zero;
const Vector4 Vector4::Point (0.0, 0.0, 0.0, 1.0);
const Vector4 Vector4::BasisX (1.0, 0.0, 0.0, 0.0);
const Vector4 Vector4::BasisY (0.0, 1.0, 0.0, 0.0);
const Vector4 Vector4::BasisZ (0.0, 0.0, 1.0, 0.0);
const Vector4 Vector4::BasisW (0.0, 0.0, 0.0, 1.0);

void Vector4::PopulateMetaType( Reflect::MetaStruct& comp )
{
	comp.AddField( &Vector4::x, "x" );
	comp.AddField( &Vector4::y, "y" );
	comp.AddField( &Vector4::z, "z" );
	comp.AddField( &Vector4::w, "w" );
}
