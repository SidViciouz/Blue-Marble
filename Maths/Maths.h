#pragma once

#include "Vector3.h"

namespace Maths
{
	void Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w);
}
