#include "PhysicsObject.h"

using namespace Physics;

PhyscisObject::PhyscisObject()
{
	mCollider = make_shared<Collider>();
}

void PhyscisObject::SetCollider(shared_ptr<Collider> collider)
{
	mCollider = collider;
}