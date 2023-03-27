#pragma once
#include "Util.h"
#include "Light.h"

#define MAX_OBJECT_NUM 1000

using namespace DirectX;

struct obj
{
	XMFLOAT4X4									world;
	XMFLOAT3									diffuseAlbedo;
	float										roughness;
	XMFLOAT3									fresnel;
	int											pad1;
};

struct env
{
	XMFLOAT4X4									view;
	XMFLOAT4X4									projection;
	Light										lights[3];
	XMFLOAT3									cameraPosition;
	int											pad1;
	XMFLOAT3									cameraFront;
	int											pad2;
	XMFLOAT4X4									invViewProjection;
	float										currentTime;
	int											pad3;
	int											pad4;
	int											pad5;
};
