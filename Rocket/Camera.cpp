#include "Camera.h"

Camera::Camera(int width,int height)
{
	
	mPosition = { 5.0f,3.0f,-5.0f };
	mUp = { 0.0f,1.0f,0.0f };
	mLookAt = { 0.0f,0.0f,0.0f };
	
	mNear = 1.0f;
	mFar = 1000.0f;
	mAngle = XMConvertToRadians(45.0f);
	mRatio = static_cast<float>(width) / height;
}

void Camera::Resize(int width, int height)
{
	mRatio = static_cast<float>(width) / height;
}

void Camera::GoFront(float a)
{

}

void Camera::GoRight(float a)
{

}