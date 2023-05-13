#pragma once

#include "../Maths/Vector3.h"
#include "PhysicsObject.h"

using namespace Maths;

namespace Physics
{
	struct Point
	{
		Vector3 p; //민코프스키 차
		Vector3 a; //A의 support function
		Vector3 b; //B의 support function
	};

	struct ContactPoint
	{
		Vector3 localA;
		Vector3 localB;
		Vector3 normal;
		float	penetration;
	};

	struct CollisionInfo
	{
		PhysicsObject* a;
		PhysicsObject* b;
		mutable int	framesLeft; 
		mutable int staticCount;

		ContactPoint point;

		void AddContactPoint(const Vector3& localA, const Vector3& localB, const Vector3& normal, float p) {
			point.localA = localA;
			point.localB = localB;
			point.normal = normal;
			point.penetration = p;
		}

		bool operator < (const CollisionInfo& other) const {
			size_t otherHash = (size_t)other.a->GetWorldID() + ((size_t)other.b->GetWorldID() << 32);
			size_t thisHash = (size_t)a->GetWorldID() + ((size_t)b->GetWorldID() << 32);

			if (thisHash < otherHash) {
				return true;
			}
			return false;
		}

		bool operator ==(const CollisionInfo& other) const {
			if (other.a == a && other.b == b) {
				return true;
			}
			return false;
		}
	};

	bool										GJKCalculation(PhysicsObject* coll1, PhysicsObject* coll2, CollisionInfo& collisionInfo);

	void										update_simplex3(Point& a, Point& b, Point& c, Point& d, int& simp_dim, Vector3& search_dir);
	bool										update_simplex4(Point& a, Point& b, Point& c, Point& d, int& simp_dim, Vector3& search_dir);

	void										EPA(Point& a, Point& b, Point& c, Point& d, PhysicsObject* coll1, PhysicsObject* coll2, CollisionInfo& collisionInfo);

	void										CalculateSearchPoint(Point& point, Vector3& search_dir, PhysicsObject* coll1, PhysicsObject* coll2);
}
