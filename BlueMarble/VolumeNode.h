#pragma once

#include "SceneNode.h"

class VolumeNode : public SceneNode
{
public:
												VolumeNode(float width,float height,float depth);
	/*
	* volume을 draw한다. (현재는 구름만 drawing하도록 되어있다. 추후 수정 예정)
	*/
	virtual void								Draw() override;
};
