#pragma once

#include "CollisionComponent.h"

class CapsuleCollisionComponent : public CollisionComponent
{
public:
												CapsuleCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo, float radius,float height);

	virtual bool								IsColliding(CollisionComponent* counterPart, CollisionInfo& collisionInfo) override;

protected:
	float										mRadius;
	float										mHeight;
};
