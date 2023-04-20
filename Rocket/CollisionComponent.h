#pragma once
#include "Util.h"
#include "Constant.h"

class SceneNode;

class CollisionComponent
{
public:
												CollisionComponent(shared_ptr<SceneNode> NodeAttachedTo);
	/*
	* other�� �浹�� �ִ� ��� true, �ƴϸ� false�� ��ȯ�Ѵ�.
	*/
	virtual bool								IsColliding(CollisionComponent* counterPart, CollisionInfo& collisionInfo) = 0;
	/*
	* collider�� �׸���.
	*/
	virtual void								Draw() = 0;
	/*
	* �� ������Ʈ�� �پ��ִ� node�� ���� �������̴�.
	*/
	shared_ptr<SceneNode>						mNodeAttachedTo;
};
