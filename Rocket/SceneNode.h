#pragma once

#include "Util.h"
#include "Math/Position.h"
#include "Math/Quaternion.h"

class SceneNode
{
public:
	string										mName;
	Position									mRelativePosition;
	Quaternion									mRelativeQuaternion;
	vector<int>									mMeshIndices;
};