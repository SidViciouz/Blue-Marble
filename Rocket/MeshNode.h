#pragma once

#include "SceneNode.h"

class MeshNode : public SceneNode
{
public:
												MeshNode(string name);

	virtual void								Draw() override;
	virtual void								DrawWithoutSetting() override;
	virtual void								Update() override;
	virtual bool								IsColliding(SceneNode* counterPart, CollisionInfo& collisionInfo) override;

	const string&								GetMeshName() const;
	void										SetActivated(const bool& value);
	const bool&									GetActivated() const;

protected:
	string										mMeshName;
	bool										mActivated = true;
};
