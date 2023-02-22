#pragma once 
#include "Util.h"

enum LightType : int
{
	Directional = 0,
	Point,
	Spot
};

class Light
{
public:
	XMFLOAT3 mPosition = { 0.0f,1.0f,0.0f };
	LightType mType = Directional;
	XMFLOAT3 mDirection{ 0.0f,-1.0f,0.0f }; // point에서는 안쓰임.
	int pad1 = 0;
	XMFLOAT3 mColor = { 1.0f,1.0f,1.0f };
	int pad2 = 0;
};

