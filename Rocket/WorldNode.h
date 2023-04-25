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

struct Point
{
	float x;
	float y;
};

struct CountryInfo
{
	struct Point geo;
	vector<vector<struct Point>> points;
};

class WorldNode : public MeshNode
{
public:
												WorldNode(string name);
	virtual void								Draw() override;
	virtual void								Update() override;
	void										MoveCharacter(const XMFLOAT3& pos);
	void										PickCountry(const XMFLOAT3& pos);

protected:
	void										UpdateCharacter();

	shared_ptr<MeshNode>						mCharacter;
	bool										isMoving = false;
	MoveInfo									mMoveInfo;

	int											mBorderTextureIdx;
	int											mBorderTextureUavIdx;
	int											mUploadBufferIdx;

	char										data[1800][3600] = { 0, };

	unordered_map<string, CountryInfo>			mCountryInfos;
	friend class								WorldInputComponent;
};
