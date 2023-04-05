#pragma once

#include "Util.h"
#include "Math/Position.h"
#include "Math/Quaternion.h"
#include "Constant.h"
#include "CollisionComponent.h"
#include "RigidBodyComponent.h"

class SceneNode
{
public:
												SceneNode();

	virtual void								Draw();
	virtual void								Update();
	bool										IsColliding(SceneNode* counterPart, CollisionInfo& collisionInfo);

	void										AddChild(shared_ptr<SceneNode> child);
	void										RemoveChild(shared_ptr<SceneNode> child);

	Position									mRelativePosition;
	Quaternion									mRelativeQuaternion;
	XMFLOAT3									mScale;

	Position									mAccumulatedPosition;
	Quaternion									mAccumulatedQuaternion;

	shared_ptr<CollisionComponent>				mCollisionComponent;
	shared_ptr<RigidBodyComponent>				mRigidBodyComponent;

	obj											mObjFeature;

	vector<shared_ptr<SceneNode>>				mChildNodes;
	SceneNode*									mParentNode = nullptr;


	int											mSceneNodeIndex;

	static bool									mSceneNodeNumTable[MAX_SCENE_NODE_NUM];
};