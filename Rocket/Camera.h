#pragma once
#include "Util.h"
#include "Entity.h"


class Camera : public Entity
{
public:
	Camera(int width, int height);
	void Resize(int width, int height);
	void GoFront(float a);
	void GoRight(float a);

	XMFLOAT3 mUp;
	XMFLOAT3 mFront;
	XMFLOAT3 mRight;

	float mNear;
	float mFar;
	float mAngle;
	float mRatio;
};
