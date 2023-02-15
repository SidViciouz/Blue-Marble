#pragma once
#include "Util.h"
#include "Light.h"

using namespace DirectX;

struct obj
{
	XMFLOAT4X4 world;
};

struct trans
{
	XMFLOAT4X4 viewProjection;
	Light lights[3];
};
