#include "Vector4.h"
#include "Vector3.h"

using namespace Maths;

Vector4::Vector4(const Vector3& v3, float newW) : x(v3.v.x), y(v3.v.y), z(v3.v.z), w(newW) {

}