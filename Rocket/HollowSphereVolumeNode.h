#pragma once

#include "SceneNode.h"

class HollowSphereVolumeNode : public SceneNode
{
public:
												HollowSphereVolumeNode(float R, float r);
	/*
	* �� �������� R�̰� ª�� �������� r�� hollow sphere volume�� �׸���.
	*/
	virtual void								Draw() override;

protected:
	float										mR;
	float										mr;
};