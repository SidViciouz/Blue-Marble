#pragma once

#include "SceneNode.h"
#include "MeshNode.h"
#include "DiceNode.h"
#include "PhysicsManager.h"

class DiceSystem : public SceneNode
{
public:
												DiceSystem(shared_ptr<PhysicsManager> physicsManager);

	/*
	* 주사위를 굴리고 나온 수의 합을 반환한다.
	*/
	int											Roll();
	/*
	* root node를 반환한다.
	*/
	shared_ptr<SceneNode>						GetRootNode() const;

protected:
	shared_ptr<SceneNode>						mRoot;
	shared_ptr<DiceNode>						mDice1;
	shared_ptr<DiceNode>						mDice2;
	shared_ptr<DiceNode>						mDice3;
	shared_ptr<MeshNode>						mGround;

	shared_ptr<PhysicsManager>					mPhysicsManager;
};
