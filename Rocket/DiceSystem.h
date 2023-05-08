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
	* �ֻ������� ������.
	*/
	void										Roll();
	/*
	* �ֻ������� �������� ����Ű�� ���� ���� ��ȯ�Ѵ�.
	*/
	int											UpperSide();
	/*
	* root node�� ��ȯ�Ѵ�.
	*/
	shared_ptr<SceneNode>						GetRootNode() const;
	
	virtual void								Update() override;

protected:
	shared_ptr<SceneNode>						mRoot;
	shared_ptr<DiceNode>						mDice1;
	shared_ptr<DiceNode>						mDice2;
	shared_ptr<DiceNode>						mDice3;
	shared_ptr<MeshNode>						mGround;

	shared_ptr<PhysicsManager>					mPhysicsManager;
};
