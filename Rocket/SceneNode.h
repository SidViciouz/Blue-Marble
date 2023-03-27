#pragma once

#include "Util.h"
#include "Math/Position.h"
#include "Math/Quaternion.h"
#include "Constant.h"

class SceneNode
{
public:
	void										Draw();
	void										Update(const XMFLOAT4X4& parentsWorld);

	string										mName;
	Position									mRelativePosition;
	Quaternion									mRelativeQuaternion;

	vector<int>									mMeshIndices;
	vector<int>									mVolumeIndices;

	vector<unique_ptr<SceneNode>>				mChildNodes;
	obj											mObjFeature;
	bool										mDirty = true;
};