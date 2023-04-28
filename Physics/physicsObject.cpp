#include "PhysicsObject.h"

using namespace Physics;

PhysicsObject::PhysicsObject()
{

}

void PhysicsObject::SetCollider(shared_ptr<Collider> collider)
{
	mCollider = collider;
}

shared_ptr<Collider> PhysicsObject::GetCollider() const
{
	return mCollider;
}

Transform& PhysicsObject::GetTransform()
{
	return mTransform;
}