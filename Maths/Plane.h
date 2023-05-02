#pragma once

#include "Vector3.h"

namespace Maths
{
	/*
	* �븻 ���Ϳ� �������κ����� �Ÿ��� ���ǵȴ�.
	*/
	class Plane
	{
	public:
												Plane();
												Plane(const Vector3& normal, float distance, bool normalise = false);

												~Plane() {}

		/*
		* �븻 ���͸� �����Ѵ�. ���� ���̿����Ѵ�.
		*/
		void									SetNormal(const Vector3& normal) { this->normal = normal; }
		/*
		* �븻 ���͸� ��´�.
		*/
		Vector3									GetNormal() const
		{ 
			return normal; 
		}
		/*
		* �������κ����� �Ÿ��� �����Ѵ�.
		*/
		void									SetDistance(float dist)
		{ 
			distance = dist;
		}
		/*
		* �������κ����� �Ÿ��� ��´�.
		*/
		float									GetDistance() const
		{ 
			return distance;
		}

		float									DistanceFromPlane(const Vector3& in) const;
		/*
		* ��� ���� �� ���� ��ȯ�Ѵ�.
		*/
		Vector3									GetPointOnPlane() const
		{
			return normal * -distance;
		}
		/*
		* ���� ������� ������ ���� ��´�.
		*/
		Vector3									ProjectPointOntoPlane(const Vector3& point) const;
		/*
		* �ﰢ�����κ��� ����� ��´�.
		*/
		static Plane							PlaneFromTri(const Vector3& v0, const Vector3& v1, const Vector3& v2);

	protected:
		/*
		* ���� ������ �븻 ����
		*/
		Vector3									normal;
		/*
		* �������κ����� �Ÿ�
		*/
		float									distance;
	};
}
