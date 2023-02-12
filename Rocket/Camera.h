#pragma once
#include "Util.h"

using namespace DirectX;

class Camera
{
public:
	XMFLOAT3 mPosition;
	XMFLOAT3 mUp;
	XMFLOAT3 mLookAt;

	float mNear;
	float mFar;
	float mAngle;
	float ratio;
};
