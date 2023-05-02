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

protected:

	vector<shared_ptr<PhysicsComponent>>		mPhysicsComponents;

	shared_ptr<PhysicsWorld>					mPhysicsWorld;

	shared_ptr<PhysicsSystem>					mPhysicsSystem;
};
