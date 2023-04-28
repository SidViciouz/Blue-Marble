#include "Collider.h"

using namespace Physics;

Collider::Collider()
{
	mColliderType = ColliderType::None;
}

const ColliderType& Collider::GetType() const
{
	return mColliderType;
}