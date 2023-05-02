#pragma once
#include "../Physics/PhysicsObject.h"
#include "PhysicsManager.h"

using namespace Physics;

class SceneNode;

class PhysicsComponent
{
	friend class PhysicsManager;

public:
												PhysicsComponent(shared_ptr<SceneNode> NodeAttachedTo,
													PhysicsType physicsType = PhysicsType::Static,
													ColliderType colliderType = ColliderType::None);
protected:
	/*
	* physics ���� �����͸� ������ �ִ� ������Ʈ�̴�.
	*/
	shared_ptr<PhysicsObject>					mPhysicsObject;
	/*
	* �� ������Ʈ�� �پ��ִ� ����̴�.
	*/
	shared_ptr<SceneNode>						mNode;

};
