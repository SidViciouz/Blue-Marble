#pragma once
#include "Util.h"

using namespace DirectX;

class Camera
{
public:
	Camera(int width, int height);
	void Resize(int width, int height);
	void GoFront(float a);
	void GoRight(float a);

	XMFLOAT3 mPosition;
	XMFLOAT3 mUp;
	XMFLOAT3 mFront;
	XMFLOAT3 mRight;

	float mNear;
	float mFar;
	float mAngle;
	float mRatio;
};
