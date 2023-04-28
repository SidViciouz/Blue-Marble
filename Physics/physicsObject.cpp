#include "PhysicsObject.h"

using namespace Physics;

PhyscisObject::PhyscisObject()
{

}

void PhyscisObject::SetCollider(shared_ptr<Collider> collider)
{
	mCollider = collider;
}