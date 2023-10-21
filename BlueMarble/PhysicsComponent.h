#pragma once
#include "../Physics/PhysicsObject.h"
#include "PhysicsManager.h"

using namespace Physics;

class SceneNode;

class PhysicsComponent
{
	friend class PhysicsManager;

public:
												PhysicsComponent(shared_ptr<SceneNode> NodeAttachedTo, PhysicsType physicsType = PhysicsType::Static);

	void										ApplyLinearImpulse(const Maths::Vector3& force);
	void										ApplyAngularImpulse(const Maths::Vector3& force);

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
