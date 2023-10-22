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
												Plane() = delete;
												Plane(const Vector3& normal, float distance, bool normalise = false);

												~Plane() {}

		float									GetDistance(const Vector3& in) const;
		/*
		* ���� ������� ������ ���� ��´�.
		*/
		Vector3									GetProjectedPoint(const Vector3& point) const;
		/*
		* �ﰢ�����κ��� ����� ��´�.
		*/
		static Plane							MakePlane(const Vector3& v0, const Vector3& v1, const Vector3& v2);

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
