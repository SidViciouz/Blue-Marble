#include "Plane.h"

using namespace Maths;


Plane::Plane(const Vector3& normal, float distance, bool normalise)
{
	if (normalise)
	{
		float length = normal.Length();

		this->normal = normal;
		this->normal.Normalize();

		this->distance = distance / length;
	}
	else
	{
		this->normal = normal;
		this->distance = distance;
	}
}

Plane Plane::MakePlane(const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
	Vector3 v1v0 = v1 - v0;
	Vector3 v2v0 = v2 - v0;

	Vector3 normal = Vector3::Cross(v1v0, v2v0);
	normal.Normalize();

	float d = -Vector3::Dot(v0, normal);

	return Plane(normal, d, false);
}

float Plane::GetDistance(const Vector3& in) const
{
	return Vector3::Dot(in, normal) + distance;
}

Vector3 Plane::GetProjectedPoint(const Vector3& point) const
{
	float distance = GetDistance(point);

	return point - (normal * distance);
}