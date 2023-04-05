#pragma once

#include "SceneNode.h"

class VolumeNode : public SceneNode
{
public:
												VolumeNode(float width,float height,float depth);
	virtual void								Draw() override;
};
