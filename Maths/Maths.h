#pragma once

namespace Maths
{
	class Vector3;
	/*
	* 삼각형의 세 꼭짓점과 그 내부의 점 하나를 입력받아서, 그 점을 barycentric coordinate으로 변환한다.
	*/
	void										Barycentric(const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& p, float& u, float& v, float& w);

	static const float							PI = 3.14159265358979323846f;

	static const float							PI_OVER_360 = PI / 360.0f;

	inline float								RadiansToDegrees(float rads)
	{
		return rads * 180.0f / PI;
	};

	inline float								DegreesToRadians(float degs)
	{
		return degs * PI / 180.0f;
	};
}
