#pragma once

#include "MeshNode.h"

class WorldNode : public MeshNode
{
public:
												WorldNode(string name);
	virtual void								Draw() override;
	virtual void								Update() override;

protected:
	shared_ptr<MeshNode>						mCharacter;

	friend class								WorldInputComponent;

};
