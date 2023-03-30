#pragma once
#include "Util.h"

class SceneNode;

class CollisionComponent
{
public:
												CollisionComponent(shared_ptr<SceneNode> NodeAttachedTo);
	virtual bool								IsColliding(CollisionComponent* counterPart) = 0;
	virtual void								Draw() = 0;

	shared_ptr<SceneNode>						mNodeAttachedTo;
};
