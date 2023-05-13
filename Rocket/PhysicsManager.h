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
	* physics component�� �����Ѵ�.
	*/
	shared_ptr<PhysicsComponent>				BuildCube(shared_ptr<SceneNode> NodeAttachedTo,
															PhysicsType physicsType,
															const Vector3& position,
															const Vector3& scale,
															float inverseMass = 5.0f,
															float elasticity = 0.8f);
	/*
	* physics component���� ������Ʈ�ϰ�, �̸� node�� �����Ѵ�.
	*/
	void										Update();

	float										GetSystemVelocity();

protected:
	/*
	* scene�� node���� physicsComponent���� �����Ѵ�.
	*/
	vector<shared_ptr<PhysicsComponent>>		mPhysicsComponents;
	/*
	* physics object���� ������ �ִ� ������Ʈ�̴�.
	*/
	shared_ptr<PhysicsWorld>					mPhysicsWorld;
	/*
	* �������� ���� ������Ʈ�� ���� ���� ������ �����ϴ� ������Ʈ�̴�.
	*/
	shared_ptr<PhysicsSystem>					mPhysicsSystem;
};
