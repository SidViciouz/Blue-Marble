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

	int											UpperSide();

protected:
	Vector3										mFace[7];
};