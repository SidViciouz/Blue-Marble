#pragma once

#include "MeshNode.h"

class DiceNode : public MeshNode
{
public:
												DiceNode(string name);
	/*
	* �� ��带 �׸���.
	*/
	virtual void								Draw() override;
};