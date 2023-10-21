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
	* physics 관련 데이터를 가지고 있는 오브젝트이다.
	*/
	shared_ptr<PhysicsObject>					mPhysicsObject;
	/*
	* 이 컴포넌트가 붙어있는 노드이다.
	*/
	shared_ptr<SceneNode>						mNode;

};
