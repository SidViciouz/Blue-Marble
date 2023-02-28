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
	void UpdateView();
	void UpdateProjection();
	void UpdateInvViewProjection();

	XMFLOAT3 mUp;
	XMFLOAT3 mFront;
	XMFLOAT3 mRight;

	float mNear;
	float mFar;
	float mAngle;
	float mRatio;

	XMFLOAT4X4 view;
	XMFLOAT4X4 projection;
	XMFLOAT4X4 invViewProjection;
};
