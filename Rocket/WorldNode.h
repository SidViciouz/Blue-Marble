#pragma once

#include "MeshNode.h"

class WorldNode : public MeshNode
{
public:
												WorldNode(string name);
	virtual void								Draw() override;
};
