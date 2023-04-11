#include "InputManager.h"
#include "InputComponent.h"
#include "SceneNode.h"
#include "Engine.h"

InputManager::InputManager()
{

}

void InputManager::Push(Message msg)
{
	mMessageQueue.push(msg);
}

void InputManager::Push(UINT msgType, int param1, int param2, int param3, int param4)
{
	mMessageQueue.push({msgType,param1,param2,param3,param4});
}

void InputManager::Clear()
{
	while (!mMessageQueue.empty())
		mMessageQueue.pop();
}

void InputManager::SetKeys(int idx,bool value)
{
	if (idx < 256)
		mKeys[idx] = value;
}

const bool& InputManager::GetKeys(int idx) const
{
	if (idx < 256)
		return mKeys[idx];

	return false;
}

void InputManager::SetMouseLeftDown(bool value)
{
	mMouseLeftDown = value;
}

bool InputManager::GetMouseLeftDown() const
{
	return mMouseLeftDown;
}

Message InputManager::Pop()
{
	if (mMessageQueue.empty())
	{
		return { 0 };
	}

	Message front = mMessageQueue.front();
	mMessageQueue.pop();

	return front;
}

void InputManager::Dispatch()
{
	for (Message msg = Pop(); msg.msgType != 0; msg = Pop())
	{
		if (msg.msgType == WM_LBUTTONDOWN)
		{
			for (auto inputComponent : mInputComponents)
			{
				if (inputComponent->IsOnMouseDownOverriden())
				{
					inputComponent->OnMouseDown(msg.param1,msg.param2);
				}
			}
			Select(msg.param1, msg.param2);
		}
		else if (msg.msgType == WM_LBUTTONUP)
		{
			for (auto inputComponent : mInputComponents)
			{
				if (inputComponent->IsOnMouseUpOverriden())
				{
					inputComponent->OnMouseUp();
				}
			}
		}
		else if (msg.msgType == WM_MOUSEMOVE)
		{
			for (auto inputComponent : mInputComponents)
			{
				if (inputComponent->IsOnMouseMoveOverriden())
				{
					inputComponent->OnMouseMove(msg.param1, msg.param2);
				}
			}
		}
	}

	
}

void InputManager::Select(int x,int y)
{
	XMFLOAT3 newPos;
	float p00 = Engine::mAllScenes[Engine::mCurrentSceneName]->envFeature.projection._11;
	float p11 = Engine::mAllScenes[Engine::mCurrentSceneName]->envFeature.projection._22;
	
	//viewport에서 view coordinate으로 변환, z = 1
	newPos.x = (2.0f * x / (float)Engine::mWidth - 1.0f)/p00;
	newPos.y = (-2.0f * y / (float)Engine::mHeight + 1.0f)/p11;
	newPos.z = 1.0f;

	//newPos를 VC에서 WC로 변환한다.
	XMMATRIX inverseViewMatrix = XMLoadFloat4x4(&Engine::mAllScenes[Engine::mCurrentSceneName]->mCameraNode->GetView());
	XMVECTOR det = XMMatrixDeterminant(inverseViewMatrix);
	inverseViewMatrix = XMMatrixInverse(&det,inverseViewMatrix);

	XMVECTOR rayVector = XMLoadFloat3(&newPos);
	XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	//XMVector3TransformCoord는 일반적인 변환이고
	//XMVector3TransformNormal은 행렬의 0~3행 중 3행을 무시한다. 따라서 translation은 할 수 없다.
	rayVector = XMVector3Normalize(XMVector3TransformNormal(rayVector, inverseViewMatrix));
	rayOrigin = XMVector3TransformCoord(rayOrigin, inverseViewMatrix);

	//clickable에서 모델을 선택한다.
	float prevDist = 1000.0f;
	float dist = 1000.0f;
	bool selected = false;

	for (auto inputComponent : mInputComponents)
	{
		if (!inputComponent->IsOnClickOverriden())
			continue;

		BoundingOrientedBox boundingBox;

		MeshNode* node = dynamic_cast<MeshNode*>(inputComponent->mNodeAttachedTo.get());

		//Mesh node여야한다.
		Engine::mMeshManager->mMeshes[node->GetMeshName()]->mBound.Transform(
			boundingBox, XMLoadFloat4x4(&node->mObjFeature.world));

		if (boundingBox.Intersects(rayOrigin, rayVector, dist))
		{
			if (dist < prevDist)
			{
				selected = true;
				inputComponent->OnClick();
				//mScenes[mCurrentScene]->mIsModelSelected = true;
				//mScenes[mCurrentScene]->mSelectedModel = model->second;
				//mScenes[mCurrentScene]->mSelectedModelName = model->first;
				prevDist = dist;
			}
		}
		
	}
}