#pragma once
#include "Util.h"
#include "Light.h"
#include "../Maths/Vector3.h"

#define MAX_OBJECT_NUM 1000

#define MAX_SCENE_NODE_NUM 1000

using namespace DirectX;
using namespace Maths;

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

struct Vertex
{
	XMFLOAT3									position;
	XMFLOAT2									uv;
	XMFLOAT3									normal;
};

struct TB
{
	XMFLOAT3									tangent;
	XMFLOAT3									bitangent;
};

struct Weight
{
	XMINT4										bone;
	XMFLOAT4									weight;
};

struct Points
{
	Vector3 p;
	Vector3 a;
	Vector3 b;
};

struct Message
{
	UINT msgType;
	int param1;
	int param2;
	int param3;
	int param4;
};

struct TextInfo
{
	int											offsetX;
	int											offsetY;
	int											width;
	int											height;
};