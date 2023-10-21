#include "Collider.h"

using namespace Physics;

AABBCollider::AABBCollider(const Vector3& halfSize)
{
	this->halfSize = halfSize;
}

Vector3 AABBCollider::Support(const Vector3& direction, const Transform& transform)
{
	//local space에서의 support
	Vector3 localDirection = transform.GetInvRotMatrix() * direction;

	Vector3 result;

	result.v.x = (localDirection.v.x > 0) ? halfSize.v.x : -halfSize.v.x;
	result.v.y = (localDirection.v.y > 0) ? halfSize.v.y : -halfSize.v.y;
	result.v.z = (localDirection.v.z > 0) ? halfSize.v.z : -halfSize.v.z;

	//world space에서의 support
	return transform.GetRotMatrix() * result + transform.GetPosition();
}