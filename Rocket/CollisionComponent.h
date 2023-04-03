#pragma once
#include "Util.h"
#include "Constant.h"

class SceneNode;

class CollisionComponent
{
public:
												CollisionComponent(shared_ptr<SceneNode> NodeAttachedTo);
	virtual bool								IsColliding(CollisionComponent* counterPart, CollisionInfo& collisionInfo) = 0;
	virtual void								Draw() = 0;

	shared_ptr<SceneNode>						mNodeAttachedTo;
};
