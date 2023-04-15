#pragma once
#include "Util.h"
#include "Constant.h"

class SceneNode;

class CollisionComponent
{
public:
												CollisionComponent(shared_ptr<SceneNode> NodeAttachedTo);
	/*
	* other과 충돌이 있는 경우 true, 아니면 false를 반환한다.
	*/
	virtual bool								IsColliding(CollisionComponent* counterPart, CollisionInfo& collisionInfo) = 0;
	/*
	* collider를 그린다.
	*/
	virtual void								Draw() = 0;
	/*
	* 이 컴포넌트가 붙어있는 node에 대한 포인터이다.
	*/
	shared_ptr<SceneNode>						mNodeAttachedTo;
};
