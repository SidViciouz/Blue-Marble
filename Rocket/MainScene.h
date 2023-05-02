#pragma once

#include "Engine.h"
#include "../Physics/PhysicsSystem.h"
#include "../Physics/PhysicsWorld.h"

//using namespace Physics;
/*
* ������ ���� scene�̴�.
*/

class InventoryNode;
class TextNode;
class ItemNode;
class WorldNode;

using namespace Physics;

class MainScene : public Scene
{
public:
												MainScene();
	/*
	* �� scene�� ���� ������Ʈ���� ���¸� �ʱ�ȭ�Ѵ�.
	*/
	void										Initialize();
	/*
	* scene�� ��� ������Ʈ���� �����Ӹ��� ������Ʈ�Ѵ�.
	*/
	virtual void								UpdateScene(const Timer& timer) override;

	virtual void								DrawScene() const override;
	
	/*
	* �� scene���� ��� ������ �ӽ÷� ����Ű�� �ִ� �������̴�.
	* ���� �����̴�.
	*/
	shared_ptr<MeshNode>						boxMesh;
	shared_ptr<MeshNode>						boxMesh2;
	shared_ptr<MeshNode>						ballMesh;
	shared_ptr<MeshNode>						groundMesh;
	shared_ptr<InventoryNode>					inventory;
	shared_ptr<WorldNode>						worldMesh;

	shared_ptr<PhysicsSystem>					mPhysicsSystem;
	shared_ptr<PhysicsWorld>					mPhysicsWorld;

	void										InitWorld();

	PhysicsObject*								AddFloorToWorld(const Vector3& position);
	PhysicsObject*								AddCubeToWorld(
																const Vector3& position,
																Vector3 dimensions,
																float inverseMass = 5.0f,
																bool bStatic = false,
																float elasticity = 0.8f);

	PhysicsObject*								mFloor;
	PhysicsObject*								mCube;
	PhysicsObject*								mCube2;
	PhysicsObject*								mCube3;
};
