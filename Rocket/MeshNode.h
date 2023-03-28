#pragma once

#include "SceneNode.h"

class MeshNode : public SceneNode
{
public:
												MeshNode(string name);

	virtual void								Draw() override;

	string										mMeshName;
};
