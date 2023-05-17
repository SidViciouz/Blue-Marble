#pragma once

#include "SceneNode.h"

class GaugeNode : public SceneNode
{
public:
												GaugeNode();

	void										SetPercentage(unsigned int pPercentage);
	unsigned int								GetPercentage() const;

	virtual void								Draw() override;
	virtual void								Update() override;

protected:
	unsigned int								mPercentage = 0;

};
