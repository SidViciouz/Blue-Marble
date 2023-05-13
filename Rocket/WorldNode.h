#pragma once

#define MAX_NUM_POINT 250

#include "MeshNode.h"
#include "AnimationNode.h"


struct MoveInfo
{
	XMFLOAT3 axis;
	float	 angle;
	float	 radius;
	float	 totalDistance;
	float	 currentDistance;
};

struct P
{
	float x;
	float y;
};

struct Country
{
	P geo;
	vector<vector<P>> points;
	P minBound;
	P maxBound;
	int index;
};


struct CountryInfo
{
	int countryIndex;
	int areaIndex;
	P minBound;
	P maxBound;
	int numOfPoint;
	P points[MAX_NUM_POINT];
};

class WorldNode : public MeshNode
{
public:
												WorldNode(string name);
	virtual void								Draw() override;
	virtual void								Update() override;
	void										MoveCharacter(const XMFLOAT3& pos);
	void										PickCountry(const XMFLOAT3& pos);
	bool										GetIsMoving() const;

protected:
	void										UpdateCharacter();
	void										LoadCountryData();

	shared_ptr<AnimatedNode> 					mCharacter;
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

	char										mData[1800][3600] = { 0, };

	unordered_map<string, Country>				mCountries;
	CountryInfo									mCountryInfos[10000];
	int											mCountryNum = 0;

	int											mLatestClicked = -2;

	friend class								WorldInputComponent;
};
