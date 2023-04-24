#pragma once

#include "MeshNode.h"

struct MoveInfo
{
	XMFLOAT3 axis;
	float	 angle;
	float	 radius;
	int		 totalFrame;
	int		 curFrame;
};

class WorldNode : public MeshNode
{
public:
												WorldNode(string name);
	virtual void								Draw() override;
	virtual void								Update() override;
	void										MoveCharacter(const XMFLOAT3& pos);

protected:
	void										UpdateCharacter();

	shared_ptr<MeshNode>						mCharacter;
	bool										isMoving = false;
	MoveInfo									mMoveInfo;
	friend class								WorldInputComponent;
};
