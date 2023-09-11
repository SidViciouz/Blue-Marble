#pragma once

#include "SceneNode.h"

class HollowSphereVolumeNode : public SceneNode
{
public:
												HollowSphereVolumeNode(float R, float r);
	/*
	* 긴 반지름이 R이고 짧은 반지름이 r인 hollow sphere volume을 그린다.
	*/
	virtual void								Draw() override;

protected:
	float										mR;
	float										mr;
};