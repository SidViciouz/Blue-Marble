#pragma once
#include "Util.h"

using namespace DirectX;

class Camera
{
public:
	Camera(int width, int height);
	void Resize(int width, int height);

	XMFLOAT3 mPosition;
	XMFLOAT3 mUp;
	XMFLOAT3 mLookAt;

	float mNear;
	float mFar;
	float mAngle;
	float mRatio;

	XMFLOAT4X4 mViewProjection ={
						1.0f,0.0f,0.0f,0.0f,
						0.0f,1.0f,0.0f,0.0f,
						0.0f,0.0f,1.0f,0.0f,
						0.0f,0.0f,0.0f,1.0f
	};
};
