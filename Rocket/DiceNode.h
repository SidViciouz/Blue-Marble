#pragma once

#include "MeshNode.h"

class DiceNode : public MeshNode
{
public:
												DiceNode(string name);
	/*
	* 이 노드를 그린다.
	*/
	virtual void								Draw() override;
};