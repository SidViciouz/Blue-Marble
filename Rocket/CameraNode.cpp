#include "CameraNode.h"
#include "Engine.h"

CameraNode::CameraNode(int width, int height)
	: SceneNode()
{
	mUp = { 0.0f,1.0f,0.0f };
	mFront = { 0.0f,0.0f,1.0f };
	mRight = { 1.0f,0.0f,0.0f };

	mNear = 1.0f;
	mFar = 3000.0f;
	mAngle = XMConvertToRadians(60.0f);
	mRatio = static_cast<float>(width) / height;

	SetRelativePosition(0.0f, 0.0f, -30.0f);
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

void CameraNode::Update()
{
	/*
	* 여기에 추가로 필요한 것들을 작성한다.
	*/
	if (mActivate)
	{
		float deltatime = Engine::mTimer.GetDeltaTime();
		//deltaTime 추가해야한다.
		if (Engine::mInputManager->GetKeys(0x57))
		{
			GoFront(10.0f * deltatime);
		}
		if (Engine::mInputManager->GetKeys(0x53))
		{
			GoFront(-10.0f * deltatime);
		}
		if (Engine::mInputManager->GetKeys(0x41))
		{
			GoRight(-10.0f * deltatime);
		}
		if (Engine::mInputManager->GetKeys(0x44))
		{
			GoRight(10.0f * deltatime);
		}

	}

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

	MulAddRelativePosition(d, mFront);
}


void CameraNode::GoRight(float d)
{
	mViewDirty = true;

	MulAddRelativePosition(d, mRight);
}

void CameraNode::TurnX(float x)
{
	mViewDirty = true;

	XMVECTOR rightAxis = XMLoadFloat3(&mRight);

	XMMATRIX R = XMMatrixRotationAxis(rightAxis, x);

	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mFront, XMVector3TransformNormal(XMLoadFloat3(&mFront), R));

	XMFLOAT4 quat;
	XMFLOAT4 curQuat = mRelativeQuaternion.Get();
	XMStoreFloat4(&quat,XMQuaternionMultiply(XMLoadFloat4(&curQuat), XMQuaternionRotationAxis(rightAxis,x)));
	SetRelativeQuaternion(quat);
}

void CameraNode::TurnY(float y)
{
	mViewDirty = true;

	XMVECTOR upAxis = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	XMMATRIX R = XMMatrixRotationY(y);

	XMStoreFloat3(&mRight, XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mFront, XMVector3TransformNormal(XMLoadFloat3(&mFront), R));

	XMFLOAT4 quat;
	XMFLOAT4 curQuat = mRelativeQuaternion.Get();
	XMStoreFloat4(&quat, XMQuaternionMultiply(XMLoadFloat4(&curQuat), XMQuaternionRotationAxis(upAxis, y)));
	SetRelativeQuaternion(quat);

	//printf("%f %f %f %f\n", quat.x, quat.y, quat.z, quat.w);
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

const XMFLOAT3& CameraNode::GetFront() const
{
	return mFront;
}


void CameraNode::Draw()
{
	SceneNode::Draw();
}

void CameraNode::SetRelativePosition(const XMFLOAT3& position)
{
	mViewDirty = true;

	SceneNode::SetRelativePosition(position);
}

void CameraNode::SetRelativePosition(const float& x, const float& y, const float& z)
{
	mViewDirty = true;

	SceneNode::SetRelativePosition(x, y, z);
}

void CameraNode::UpdateViewMatrix()
{
	if (!mViewDirty)
		return;

	mViewDirty = false;
	
	XMVECTOR right = XMLoadFloat3(&mRight);
	XMVECTOR up = XMLoadFloat3(&mUp);
	XMVECTOR front = XMVector3Normalize(XMLoadFloat3(&mFront));
	XMFLOAT3 pos = GetRelativePosition().v;
	XMVECTOR position = XMLoadFloat3(&pos);

	up = XMVector3Normalize(XMVector3Cross(front, right));
	right = XMVector3Cross(up, front);

	XMStoreFloat3(&mRight, right);
	XMStoreFloat3(&mUp, up);
	XMStoreFloat3(&mFront, front);

	float x = -XMVectorGetX(XMVector3Dot(position, right));
	float y = -XMVectorGetX(XMVector3Dot(position, up));
	float z = -XMVectorGetX(XMVector3Dot(position, front));


	mView = {
		mRight.x, mUp.x, mFront.x, 0.0f,
		mRight.y, mUp.y, mFront.y, 0.0f,
		mRight.z, mUp.z, mFront.z, 0.0f,
		x,y,z,1.0f
	};
	
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

void CameraNode::ToggleActivate()
{
	if (mActivate)
		mActivate = false;
	else
		mActivate = true;
}

const bool& CameraNode::GetActivate() const
{
	return mActivate;
}

void CameraNode::LookFront()
{
	mViewDirty = true;

	mUp = { 0.0f,1.0f,0.0f };
	mFront = { 0.0f,0.0f,1.0f };
	mRight = { 1.0f,0.0f,0.0f };
}

void CameraNode::LookDown()
{
	mViewDirty = true;

	mUp = { 0.0f,0.0f,1.0f };
	mFront = { 0.0f,-1.0f,0.0f };
	mRight = { 1.0f,0.0f,0.0f };
}

void CameraNode::AddAngle(const float& pAngle)
{
	mProjectionDirty = true;

	mAngle += XMConvertToRadians(pAngle);
}

void CameraNode::SetAngle(const float& pAngle)
{
	mProjectionDirty = true;

	mAngle = XMConvertToRadians(pAngle);
}