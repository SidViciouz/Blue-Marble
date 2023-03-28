#pragma once

#include "CollisionComponent.h"

class SphereCollisionComponent : public CollisionComponent
{
public:
	SphereCollisionComponent(shared_ptr<SceneNode> NodeAttachedTo,float radius);

	/*
	* sphere to sphere collsion test¿Ã¥Ÿ.
	*/
	virtual bool								IsColliding(CollisionComponent* counterPart) override;

protected:
	float										mRadius;
};
