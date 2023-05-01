#pragma once

#include "Collider.h"

namespace Physics
{
	class AABBCollider : Collider
	{
	public:
		AABBCollider(const Vector3& halfDims) {
			mType = ColliderType::AABB;
			halfSizes = halfDims;
		}
		~AABBCollider() {

		}

		Vector3 GetHalfDimensions() const {
			return halfSizes;
		}

		Vector3 Support(const Vector3& dir, const Transform& transform) {
			Vector3 localDir = transform.GetInvRotMatrix() * dir; //find support in model space

			Vector3 result;
			result.v.x = (localDir.v.x > 0) ? halfSizes.v.x : -halfSizes.v.x;
			result.v.y = (localDir.v.y > 0) ? halfSizes.v.y : -halfSizes.v.y;
			result.v.z = (localDir.v.z > 0) ? halfSizes.v.z : -halfSizes.v.z;

			return transform.GetRotMatrix() * result + transform.GetPosition(); //convert support to world space
			//Ray r(Vector3(0, 0, 0), dir);
			//RayCollision collision;
			//CollisionDetection::RayOBBIntersection(r, transform, *this, collision);
			//return collision.collidedAt;
		}

	protected:
		Vector3 halfSizes;
	};
}
