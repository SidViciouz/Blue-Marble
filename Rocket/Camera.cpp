#include "Camera.h"

Camera::Camera(int width,int height)
{
	mPosition.Set(0.0f,0.0f,-5.0f);
	mUp = { 0.0f,1.0f,0.0f };
	mFront = { 0.0f,0.0f,1.0f };
	mRight = { 1.0f,0.0f,0.0f };
	
	mNear = 1.0f;
	mFar = 1000.0f;
	mAngle = XMConvertToRadians(45.0f);
	mRatio = static_cast<float>(width) / height;

	UpdateView();
	UpdateProjection();
}

void Camera::Resize(int width, int height)
{
	mRatio = static_cast<float>(width) / height;
}

void Camera::GoFront(float a)
{
	mPosition.MulAdd(a, mFront);
}

void Camera::GoRight(float a)
{
	mPosition.MulAdd(a, mRight);
}

void Camera::UpdateView()
{
	XMVECTOR right = XMLoadFloat3(&mRight);
	XMVECTOR up = XMLoadFloat3(&mUp);
	XMVECTOR front = XMVector3Normalize(XMLoadFloat3(&mFront));
	XMVECTOR position = XMLoadFloat3(&mPosition.Get());

	up = XMVector3Normalize(XMVector3Cross(front, right));
	right = XMVector3Cross(up, front);

	XMStoreFloat3(&mRight, right);
	XMStoreFloat3(&mUp, up);
	XMStoreFloat3(&mFront, front);

	float x = -XMVectorGetX(XMVector3Dot(position, right));
	float y = -XMVectorGetX(XMVector3Dot(position, up));
	float z = -XMVectorGetX(XMVector3Dot(position, front));

	view = {
		mRight.x, mUp.x, mFront.x, 0.0f,
		mRight.y, mUp.y, mFront.y, 0.0f,
		mRight.z, mUp.z, mFront.z, 0.0f,
		x,y,z,1.0f
	};
}

void Camera::UpdateProjection()
{
	XMStoreFloat4x4(&projection, XMMatrixPerspectiveFovLH(mAngle, mRatio, mNear, mFar));
}