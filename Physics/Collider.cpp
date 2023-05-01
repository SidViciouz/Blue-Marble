#include "Collider.h"

using namespace Physics;

Collider::Collider()
{
	mType = ColliderType::None;
}

const ColliderType& Collider::GetType() const
{
	return mType;
}