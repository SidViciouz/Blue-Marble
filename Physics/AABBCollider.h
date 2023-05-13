#pragma once

#include "Collider.h"

namespace Physics
{
	class AABBCollider : Collider
	{
	public:
		AABBCollider(const Vector3& halfDims)
		{
			mType = ColliderType::AABB;
			halfSizes = halfDims;
		}
		~AABBCollider() {

		}

		Vector3 GetHalfDimensions() const
		{
			return halfSizes;
		}

		Vector3 Support(const Vector3& dir, const Transform& transform)
		{
			//local space에서의 support
			Vector3 localDir = transform.GetInvRotMatrix() * dir;

			Vector3 result;
			result.v.x = (localDir.v.x > 0) ? halfSizes.v.x : -halfSizes.v.x;
			result.v.y = (localDir.v.y > 0) ? halfSizes.v.y : -halfSizes.v.y;
			result.v.z = (localDir.v.z > 0) ? halfSizes.v.z : -halfSizes.v.z;

			//world space에서의 support
			return transform.GetRotMatrix() * result + transform.GetPosition();
		}

	protected:
		Vector3 halfSizes;
	};
}
