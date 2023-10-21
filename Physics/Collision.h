#pragma once

#include "../Maths/Vector3.h"
#include "PhysicsObject.h"

using namespace Maths;

namespace Physics
{
	struct Point
	{
		Vector3 difference; //민코프스키 차
		Vector3 a; //A의 support function
		Vector3 b; //B의 support function
	};

	struct ContactInfo
	{
		Vector3 a;
		Vector3 b;
		Vector3 normal;
		float	depth;
	};

	struct CollisionInfo
	{
		PhysicsObject* a;
		PhysicsObject* b;

		ContactInfo mInfo;

		void AddContactInfo(const Vector3& a, const Vector3& b, const Vector3& normal, float depth)
		{
			mInfo.a = a;
			mInfo.b = b;
			mInfo.normal = normal;
			mInfo.depth = depth;
		}
	};

	bool										GJK(CollisionInfo& collisionInfo);

	void										EPA(Point& a, Point& b, Point& c, Point& d, CollisionInfo& collisionInfo);

	void										SearchPoint(Point& point, Vector3& searchDirection, PhysicsObject* objectA, PhysicsObject* objectB);

	void										UpdateSimplex3(Point& a, Point& b, Point& c, Point& d, int& dimension, Vector3& searchDirection);

	bool										UpdateSimplex4(Point& a, Point& b, Point& c, Point& d, Vector3& searchDirection);
}
