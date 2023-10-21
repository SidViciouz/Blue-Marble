#include "PhysicsComponent.h"

PhysicsComponent::PhysicsComponent(shared_ptr<SceneNode> NodeAttachedTo, PhysicsType physicsType)
	: mNode(NodeAttachedTo)
{
	mPhysicsObject = make_shared<PhysicsObject>(physicsType);
}

void PhysicsComponent::ApplyLinearImpulse(const Maths::Vector3& force)
{
	mPhysicsObject->ApplyLinearImpulse(force);
}

void PhysicsComponent::ApplyAngularImpulse(const Maths::Vector3& force)
{
	mPhysicsObject->ApplyAngularImpulse(force);
}