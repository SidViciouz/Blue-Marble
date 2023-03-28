#pragma once

#include "Util.h"
#include "Math/Position.h"
#include "Math/Quaternion.h"
#include "Constant.h"

class SceneNode
{
public:
												SceneNode();

	virtual void								Draw();
	virtual void								Update(const XMFLOAT4X4& parentsWorld);

	void										AddChild(unique_ptr<SceneNode> child);
	void										RemoveChild(unique_ptr<SceneNode> child);

	Position									mRelativePosition;
	Quaternion									mRelativeQuaternion;

	obj											mObjFeature;

	vector<unique_ptr<SceneNode>>				mChildNodes;

	bool										mDirty = true;

	int											mSceneNodeIndex;

	static bool									mSceneNodeNumTable[MAX_SCENE_NODE_NUM];
};