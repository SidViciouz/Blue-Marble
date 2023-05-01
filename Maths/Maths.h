#pragma once

namespace Maths
{
	class Vector3;

	void Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w);

	static const float		PI = 3.14159265358979323846f;

	//It's pi...divided by 360.0f!
	static const float		PI_OVER_360 = PI / 360.0f;

	//Radians to degrees
	inline float RadiansToDegrees(float rads) {
		return rads * 180.0f / PI;
	};

	//Degrees to radians
	inline float DegreesToRadians(float degs) {
		return degs * PI / 180.0f;
	};
}
