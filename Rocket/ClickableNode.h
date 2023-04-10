#pragma once
#include "MeshNode.h"

class ClickableNode : public MeshNode
{
public:
												ClickableNode(string name);
	virtual void								Draw() override;
	virtual void								Update() override;

protected:
};
