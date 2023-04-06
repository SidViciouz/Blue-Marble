#include "ClickableNode.h"
#include "Engine.h"

ClickableNode::ClickableNode(string name)
	: MeshNode(name)
{
}

void ClickableNode::Draw()
{
	MeshNode::Draw();
}

void ClickableNode::Update()
{
	for (Message msg = Engine::mInputManager->Pop(); msg.msgType != 0; msg = Engine::mInputManager->Pop())
	{
		if (msg.msgType == WM_LBUTTONDOWN && mMouseDown == false)
		{
			mMouseDown = true;

			if(IsMouseOn(msg.param1,msg.param2))
			{
				OnClick();
			}
		}
		else if (msg.msgType == WM_LBUTTONUP)
		{
			mMouseDown = false;
		}
		else if (msg.msgType == WM_MOUSEMOVE)
		{

		}
	}


	MeshNode::Update();
}

bool ClickableNode::IsMouseOn(int x, int y) const
{
	return false;
	/*
	XMFLOAT3 newPos;
	float p00 = envFeature.projection._11;
	float p11 = envFeature.projection._22;

	//viewport에서 view coordinate으로 변환, z = 1
	newPos.x = (2.0f * x / (float)mWidth - 1.0f) / p00;
	newPos.y = (-2.0f * y / (float)mHeight + 1.0f) / p11;
	newPos.z = 1.0f;

	//newPos를 VC에서 WC로 변환한다.
	XMMATRIX inverseViewMatrix = XMLoadFloat4x4(&mCamera->view);
	XMVECTOR det = XMMatrixDeterminant(inverseViewMatrix);
	inverseViewMatrix = XMMatrixInverse(&det, inverseViewMatrix);

	XMVECTOR rayVector = XMLoadFloat3(&newPos);
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	//XMVector3TransformCoord는 일반적인 변환이고
	//XMVector3TransformNormal은 행렬의 0~3행 중 3행을 무시한다. 따라서 translation은 할 수 없다.
	rayVector = XMVector3Normalize(XMVector3TransformNormal(rayVector, inverseViewMatrix));
	rayOrigin = XMVector3TransformCoord(rayOrigin, inverseViewMatrix);

	//clickable에서 모델을 선택한다.
	float prevDist = 1000.0f;
	float dist = 1000.0f;
	for (auto model = mScenes[mCurrentScene]->mModels->begin(); model != mScenes[mCurrentScene]->mModels->end(); model++)
	{
		BoundingOrientedBox boundingBox;

		model->second->mBound.Transform(boundingBox, XMLoadFloat4x4(&model->second->mObjFeature.world));

		if (boundingBox.Intersects(rayOrigin, rayVector, dist))
		{
			if (dist < prevDist)
			{
				mIsModelSelected = true;
				mSelectedModel = model->second;
				mSelectedModelName = model->first;
				prevDist = dist;
			}
		}
	}
	*/
}

void ClickableNode::OnClick()
{

}