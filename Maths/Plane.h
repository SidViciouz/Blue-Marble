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
												Plane();
												Plane(const Vector3& normal, float distance, bool normalise = false);

												~Plane() {}

		/*
		* 노말 벡터를 설정한다. 단위 길이여야한다.
		*/
		void									SetNormal(const Vector3& normal) { this->normal = normal; }
		/*
		* 노말 벡터를 얻는다.
		*/
		Vector3									GetNormal() const
		{ 
			return normal; 
		}
		/*
		* 원점으로부터의 거리를 설정한다.
		*/
		void									SetDistance(float dist)
		{ 
			distance = dist;
		}
		/*
		* 원점으로부터의 거리를 얻는다.
		*/
		float									GetDistance() const
		{ 
			return distance;
		}

		float									DistanceFromPlane(const Vector3& in) const;
		/*
		* 평면 위의 한 점을 반환한다.
		*/
		Vector3									GetPointOnPlane() const
		{
			return normal * -distance;
		}
		/*
		* 점을 평면위에 투영한 점을 얻는다.
		*/
		Vector3									ProjectPointOntoPlane(const Vector3& point) const;
		/*
		* 삼각형으로부터 평면을 얻는다.
		*/
		static Plane							PlaneFromTri(const Vector3& v0, const Vector3& v1, const Vector3& v2);

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
