#include "Camera.h"

Camera::Camera(int width,int height)
{
	mPosition = { 0.0f,10.0f,0.0f };
	mUp = { 0.0f,1.0f,0.0f };
	mLookAt = { 0.0f,0.0f,20.0f };

	mNear = 1.0f;
	mFar = 1000.0f;
	mAngle = 45.0f;
	ratio = static_cast<float>(width) / height;
}

void Camera::Resize(int width, int height)
{
	ratio = static_cast<float>(width) / height;
}