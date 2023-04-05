#include "CameraNode.h"

CameraNode::CameraNode(int width, int height)
	: SceneNode()
{
	mNear = 1.0f;
	mFar = 1000.0f;
	mAngle = XMConvertToRadians(45.0f);
	mRatio = static_cast<float>(width) / height;

	SetRelativeQuaternion(0.0f,cosf(XMConvertToRadians(-45.0f)),0.0f,sinf(XMConvertToRadians(-45.0f)));
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void CameraNode::Update()
{
	UpdateViewMatrix();
	UpdateProjectionMatrix();

	SceneNode::Update();
}

void CameraNode::Resize(int width, int height)
{
	mProjectionDirty = true;

	mRatio = static_cast<float>(width) / height;
}

void CameraNode::GoFront(float d)
{
	mViewDirty = true;

	float angle;
	XMVECTOR axis;
	XMVECTOR quat = XMLoadFloat4(&mRelativeQuaternion.Get());

	XMQuaternionToAxisAngle(&axis, &angle, quat);
	XMVECTOR perp = XMVector3Orthogonal(axis);

	XMVECTOR front = XMVector3Normalize(XMVector3Rotate(perp, quat));

	XMFLOAT3 frontDir;
	XMStoreFloat3(&frontDir, front);

	printf("front : %f %f %f\n", frontDir.x, frontDir.y, frontDir.z);

	MulAddRelativePosition(d, frontDir);
}

void CameraNode::GoRight(float d)
{
	mViewDirty = true;

	float angle;
	XMVECTOR axis;
	XMVECTOR quat = XMLoadFloat4(&mRelativeQuaternion.Get());

	XMQuaternionToAxisAngle(&axis, &angle, quat);
	XMVECTOR perp = XMVector3Orthogonal(axis);

	XMVECTOR quatRight = XMQuaternionRotationAxis(axis, XMConvertToRadians(90.0f));

	quat = XMQuaternionMultiply(quat, quatRight);

	XMVECTOR right = XMVector3Normalize(XMVector3Rotate(perp, quat));

	XMFLOAT3 rightDir;
	XMStoreFloat3(&rightDir, right);

	MulAddRelativePosition(d, rightDir);
}

void CameraNode::Turn(float x, float y)
{
	mViewDirty = true;

	XMVECTOR xAxis = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR yAxis = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	float xAngle = XMConvertToRadians(-y/10.0f);
	float yAngle = XMConvertToRadians(x/10.0f);

	XMVECTOR xRotation = XMQuaternionRotationAxis(xAxis, xAngle);
	XMVECTOR yRotation = XMQuaternionRotationAxis(yAxis, yAngle);

	XMVECTOR finalRotation = XMQuaternionMultiply(xRotation, yRotation);
	XMFLOAT4 dQuatDir;
	XMStoreFloat4(&dQuatDir, finalRotation);

	float angle;
	XMVECTOR axis;
	XMVECTOR quat = XMLoadFloat4(&mRelativeQuaternion.Get());
	XMQuaternionToAxisAngle(&axis, &angle, quat);
	printf("axis : %f %f %f, angle : %f\n", XMVectorGetX(axis), XMVectorGetY(axis), XMVectorGetZ(axis), XMConvertToDegrees(angle));

	MulRelativeQuaternion(dQuatDir);

	quat = XMLoadFloat4(&mRelativeQuaternion.Get());
	XMQuaternionToAxisAngle(&axis, &angle, quat);
	printf("axis : %f %f %f, angle : %f\n", XMVectorGetX(axis), XMVectorGetY(axis), XMVectorGetZ(axis), XMConvertToDegrees(angle));
}

const XMFLOAT4X4& CameraNode::GetView() const
{
	return mView;
}

const XMFLOAT4X4& CameraNode::GetProjection() const
{
	return mProjection;
}

const XMFLOAT4X4& CameraNode::GetInvVIewProjection() const
{
	return mInvViewProjection;
}

XMFLOAT3 CameraNode::GetFront() const
{
	float angle;
	XMVECTOR axis;
	XMVECTOR quat = XMLoadFloat4(&mRelativeQuaternion.Get());

	XMQuaternionToAxisAngle(&axis, &angle, quat);
	XMVECTOR perp = XMVector3Orthogonal(axis);

	XMVECTOR front = XMVector3Normalize(XMVector3Rotate(perp, quat));

	XMFLOAT3 frontDir;
	XMStoreFloat3(&frontDir, front);

	return frontDir;
}


void CameraNode::Draw()
{

}

void CameraNode::UpdateViewMatrix()
{
	if (!mViewDirty)
		return;

	mViewDirty = false;

	XMMATRIX rotationM =  XMMatrixRotationQuaternion(XMLoadFloat4(&mRelativeQuaternion.Get()));
	XMMATRIX translationM = XMMatrixTranslationFromVector(XMLoadFloat3(&mRelativePosition.Get()));

	XMStoreFloat4x4(&mView, XMMatrixMultiply(rotationM, translationM));
	//XMStoreFloat4x4(&mView, translationM);

	XMMATRIX v = XMLoadFloat4x4(&mView);
	XMMATRIX p = XMLoadFloat4x4(&mProjection);

	XMMATRIX vp = XMMatrixMultiply(v, p);
	XMStoreFloat4x4(&mViewProjection, vp);
	XMStoreFloat4x4(&mInvViewProjection, XMMatrixInverse(nullptr, vp));
}

void CameraNode::UpdateProjectionMatrix()
{
	if (!mProjectionDirty)
		return;

	mProjectionDirty = false;

	XMStoreFloat4x4(&mProjection, XMMatrixPerspectiveFovLH(mAngle, mRatio, mNear, mFar));

	XMMATRIX v = XMLoadFloat4x4(&mView);
	XMMATRIX p = XMLoadFloat4x4(&mProjection);

	XMMATRIX vp = XMMatrixMultiply(v, p);
	XMStoreFloat4x4(&mViewProjection, vp);
	XMStoreFloat4x4(&mInvViewProjection, XMMatrixInverse(nullptr, vp));
}