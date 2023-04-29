#include "Maths.h"
#include "Vector3.h"

void Maths::Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w)
{
	Vector3 v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = Vector3::Dot(v0, v0);
	float d01 = Vector3::Dot(v0, v1);
	float d11 = Vector3::Dot(v1, v1);
	float d20 = Vector3::Dot(v2, v0);
	float d21 = Vector3::Dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;

}