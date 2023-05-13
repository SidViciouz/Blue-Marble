#pragma once

#include <memory>
#include <vector>
#include "../Physics/PhysicsObject.h"
#include "../Physics/PhysicsWorld.h"
#include "../Physics/PhysicsSystem.h"

using namespace Physics;
using namespace std;

class PhysicsComponent;
class SceneNode;

class PhysicsManager
{
public:
												PhysicsManager();
	/*
	* physics component를 생성한다.
	*/
	shared_ptr<PhysicsComponent>				BuildCube(shared_ptr<SceneNode> NodeAttachedTo,
															PhysicsType physicsType,
															const Vector3& position,
															const Vector3& scale,
															float inverseMass = 5.0f,
															float elasticity = 0.8f);
	/*
	* physics component들을 업데이트하고, 이를 node에 적용한다.
	*/
	void										Update();

	float										GetSystemVelocity();

protected:
	/*
	* scene의 node들의 physicsComponent들을 저장한다.
	*/
	vector<shared_ptr<PhysicsComponent>>		mPhysicsComponents;
	/*
	* physics object들을 가지고 있는 오브젝트이다.
	*/
	shared_ptr<PhysicsWorld>					mPhysicsWorld;
	/*
	* 물리적인 상태 업데이트등 물리 관련 로직을 수행하는 오브젝트이다.
	*/
	shared_ptr<PhysicsSystem>					mPhysicsSystem;
};
