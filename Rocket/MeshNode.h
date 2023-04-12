#pragma once

#include "SceneNode.h"

class MeshNode : public SceneNode
{
public:
												MeshNode(string name);

	virtual void								Draw() override;
	virtual void								DrawWithoutSetting() override;
	virtual void								Update() override;
	const string&								GetMeshName() const;

protected:
	string										mMeshName;
};
