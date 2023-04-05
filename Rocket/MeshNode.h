#pragma once

#include "SceneNode.h"

class MeshNode : public SceneNode
{
public:
												MeshNode(string name);

	virtual void								Draw() override;
	virtual void								Update() override;

	string										mMeshName;
};
