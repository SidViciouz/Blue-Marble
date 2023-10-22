#pragma once

#include "Vector3.h"

namespace Maths
{
	/*
	* 노말 벡터와 원점으로부터의 거리로 정의된다.
	*/
	class Plane
	{
	public:
												Plane() = delete;
												Plane(const Vector3& normal, float distance, bool normalise = false);

												~Plane() {}

		float									GetDistance(const Vector3& in) const;
		/*
		* 점을 평면위에 투영한 점을 얻는다.
		*/
		Vector3									GetProjectedPoint(const Vector3& point) const;
		/*
		* 삼각형으로부터 평면을 얻는다.
		*/
		static Plane							MakePlane(const Vector3& v0, const Vector3& v1, const Vector3& v2);

	protected:
		/*
		* 단위 길이의 노말 벡터
		*/
		Vector3									normal;
		/*
		* 원점으로부터의 거리
		*/
		float									distance;
	};
}
