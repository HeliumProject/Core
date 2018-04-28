#include "Precompile.h"
#include "Math/Vector3.h"
#include "Math/Scale.h"
#include "Reflect/TranslatorDeduction.h"

HELIUM_DEFINE_BASE_STRUCT( Helium::Vector3 );

using namespace Helium;

const Vector3 Vector3::Zero;
const Vector3 Vector3::Unit   (1.0, 1.0, 1.0);
const Vector3 Vector3::BasisX (1.0, 0.0, 0.0);
const Vector3 Vector3::BasisY (0.0, 1.0, 0.0);
const Vector3 Vector3::BasisZ (0.0, 0.0, 1.0);

void Vector3::PopulateMetaType( Reflect::MetaStruct& comp )
{
	comp.AddField( &Vector3::x, "x" );
	comp.AddField( &Vector3::y, "y" );
	comp.AddField( &Vector3::z, "z" );
}

Vector3 Vector3::operator*(const Scale& v) const
{
	return Vector3 (x * v.x, y * v.y, z * v.z);
}
