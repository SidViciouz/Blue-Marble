#include "PhysicsComponent.h"

PhysicsComponent::PhysicsComponent(shared_ptr<SceneNode> NodeAttachedTo,
	PhysicsType physicsType,
	ColliderType colliderType)
	: mNode(NodeAttachedTo)
{
	mPhysicsObject = make_shared<PhysicsObject>(physicsType);
}