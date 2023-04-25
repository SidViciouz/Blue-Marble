#pragma once

#define MAX_NUM_POINT 250

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

struct Country
{
	struct Point geo;
	vector<vector<struct Point>> points;
	struct Point minBound;
	struct Point maxBound;
	int index;
};


struct CountryInfo
{
	int countryIndex;
	struct Point minBound;
	struct Point maxBound;
	int numOfPoint;
	struct Point points[MAX_NUM_POINT];
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

	int											mColorCountryTextureIdx;
	int											mColorCountryTextureUavIdx;

	int											mCountryInfoBufferIdx;
	int											mCountryInfoBufferSrvIdx;
	int											mCountryInfoUploadBufferIdx;

	char										data[1800][3600] = { 0, };

	unordered_map<string, Country>				mCountrys;

	CountryInfo									mCountryInfos[10000];

	friend class								WorldInputComponent;
};
