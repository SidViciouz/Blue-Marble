#pragma once
#include "Util.h"
#include "Light.h"

using namespace DirectX;

struct obj
{
	XMFLOAT4X4 world;
	XMFLOAT3 diffuseAlbedo;
	float roughness;
	XMFLOAT3 fresnel;
	int pad1;
};

struct trans
{
	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	Light lights[3];
	XMFLOAT3 cameraPosition;
	int pad1;
};
