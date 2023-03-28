#pragma once

#include "SceneNode.h"

class VolumeNode : public SceneNode
{
public:
												VolumeNode(float width,float height,float depth);
	virtual void								Draw() override;
	virtual void								Update(const XMFLOAT4X4& parentsWorld) override;

protected:
	XMFLOAT3									mScale;
};
