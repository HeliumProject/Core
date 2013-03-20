#include "MathPch.h"
#include "Math/Vector3.h"
#include "Math/Scale.h"

using namespace Helium;

const Vector3 Vector3::Zero;
const Vector3 Vector3::Unit   (1.0, 1.0, 1.0);
const Vector3 Vector3::BasisX (1.0, 0.0, 0.0);
const Vector3 Vector3::BasisY (0.0, 1.0, 0.0);
const Vector3 Vector3::BasisZ (0.0, 0.0, 1.0);

Vector3  Vector3::operator* (const Scale& v) const { return Vector3 (x * v.x, y * v.y, z * v.z); }

using namespace Helium;
