#pragma once

#include "Util.h"
#include "Math/Position.h"
#include "Math/Quaternion.h"
#include "Constant.h"

class SceneNode
{
public:
												SceneNode();

	void										Draw();
	void										Update(const XMFLOAT4X4& parentsWorld);

	string										mName;
	Position									mRelativePosition;
	Quaternion									mRelativeQuaternion;

	obj											mObjFeature;

	vector<string>								mMeshNames;
	vector<string>								mVolumeNames;

	vector<unique_ptr<SceneNode>>				mChildNodes;

	bool										mDirty = true;

	int											mSceneNodeIndex;

	static bool									mSceneNodeNumTable[MAX_SCENE_NODE_NUM];
};