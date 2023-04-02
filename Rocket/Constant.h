#pragma once
#include "Util.h"
#include "Light.h"

#define MAX_OBJECT_NUM 1000

#define MAX_SCENE_NODE_NUM 1000

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

struct Vertex
{
	XMFLOAT3									position;
	XMFLOAT2									uv;
	XMFLOAT3									normal;
};

struct Vector3
{
												Vector3()
	{
		this->v.x = 0.0f;
		this->v.y = 0.0f;
		this->v.z = 0.0f;
	}
												Vector3(const float& x,const float& y,const float& z)
	{
		this->v.x = x;
		this->v.y = y;
		this->v.z = z;
	}

	XMFLOAT3									v;

	Vector3										operator+(const Vector3& other) const
	{
		Vector3 result;
		
		XMStoreFloat3(&result.v,XMLoadFloat3(&v) + XMLoadFloat3(&other.v));

		return result;
	}
	Vector3										operator-(const Vector3& other) const
	{
		Vector3 result;

		XMStoreFloat3(&result.v, XMLoadFloat3(&v) - XMLoadFloat3(&other.v));
		
		return result;
	}
	Vector3										operator*(const float& c) const
	{
		Vector3 result;

		XMStoreFloat3(&result.v, XMLoadFloat3(&v) * c);

		return result;
	}
	float										operator*(const Vector3& other) const
	{
		float result;

		result = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&v), XMLoadFloat3(&other.v)));

		return result;
	}
	Vector3										operator*(const XMFLOAT3X3& m) const
	{
		Vector3 result;
		
		XMStoreFloat3(&result.v, XMVector3Transform(XMLoadFloat3(&v), XMLoadFloat3x3(&m)));

		return result;
	}
	Vector3										operator/(const float& c) const
	{
		if (fabs(c) < 0.00001f)
			return Vector3(0.0f, 0.0f, 0.0f);

		Vector3 result;

		XMStoreFloat3(&result.v, XMLoadFloat3(&v) / c);

		return result;
	}
	/*
	* cross product
	*/
	Vector3										operator^(const Vector3& other) const
	{
		Vector3 result;

		XMStoreFloat3(&result.v, XMVector3Cross(XMLoadFloat3(&v), XMLoadFloat3(&other.v)));

		return result;
	}
	float										length()
	{
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}
	Vector3										normalize()
	{
		Vector3 result;

		XMStoreFloat3(&result.v, XMLoadFloat3(&v) / length());

		return result;
	}
};


struct CollisionInfo
{
	bool isIntersected = false;
	float depth = 0.0f;
	Vector3 normal;
	Vector3 point;
};

struct Closest
{
	int		faceIdx;
	float	distance;
};