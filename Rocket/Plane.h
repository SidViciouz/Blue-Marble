#pragma once

#include "Constant.h"

class Plane {
public:
	Plane(void);
	Plane(const Vector3& normal, float distance, bool normalise = false);

	~Plane(void) {};

	//Sets the planes normal, which should be UNIT LENGTH!!!
	void	SetNormal(const Vector3& normal) { this->normal = normal; }
	//Gets the planes normal.
	Vector3 GetNormal() const { return normal; }
	//Sets the planes distance from the origin
	void	SetDistance(float dist) { distance = dist; }
	//Gets the planes distance from the origin
	float	GetDistance() const { return distance; }
	//Performs a simple sphere / plane test
	bool SphereInPlane(const Vector3& position, float radius) const;
	//Performs a simple sphere / point test
	bool PointInPlane(const Vector3& position) const;

	float	DistanceFromPlane(const Vector3& in) const;

	Vector3 GetPointOnPlane() const {
		return normal * -distance;
	}

	Vector3 ProjectPointOntoPlane(const Vector3& point) const;


	static Plane PlaneFromTri(const Vector3& v0, const Vector3& v1, const Vector3& v2);

protected:
	//Unit-length plane normal
	Vector3 normal;
	//Distance from origin
	float	distance;
};
