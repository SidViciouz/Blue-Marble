#pragma once

#include "InputComponent.h"
#include "Engine.h"
#include "WorldNode.h"

class WorldInputComponent : public InputComponent<WorldInputComponent>
{
public:
	WorldInputComponent(shared_ptr<SceneNode> NodeAttachedTo)
		: InputComponent<WorldInputComponent>(NodeAttachedTo)
	{
		mNode = dynamic_cast<WorldNode*>(mNodeAttachedTo.get());
	}

	virtual void								OnMouseDown(const int& x, const int& y) override
	{
		prevX = x;
		prevY = y;

		XMFLOAT3 newPos;
		float p00 = Engine::mAllScenes[Engine::mCurrentSceneName]->envFeature.projection._11;
		float p11 = Engine::mAllScenes[Engine::mCurrentSceneName]->envFeature.projection._22;

		//viewport에서 view coordinate으로 변환, z = 1
		newPos.x = (2.0f * x / (float)Engine::mWidth - 1.0f) / p00;
		newPos.y = (-2.0f * y / (float)Engine::mHeight + 1.0f) / p11;
		newPos.z = 1.0f;

		//newPos를 VC에서 WC로 변환한다.
		XMMATRIX inverseViewMatrix = XMLoadFloat4x4(&Engine::mAllScenes[Engine::mCurrentSceneName]->mCameraNode->GetView());
		XMVECTOR det = XMMatrixDeterminant(inverseViewMatrix);
		inverseViewMatrix = XMMatrixInverse(&det, inverseViewMatrix);

		XMVECTOR rayVector = XMLoadFloat3(&newPos);
		XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		//XMVector3TransformCoord는 일반적인 변환이고
		//XMVector3TransformNormal은 행렬의 0~3행 중 3행을 무시한다. 따라서 translation은 할 수 없다.
		rayVector = XMVector3Normalize(XMVector3TransformNormal(rayVector, inverseViewMatrix));
		rayOrigin = XMVector3TransformCoord(rayOrigin, inverseViewMatrix);

		XMFLOAT3 pos = mNode->GetAccumulatedPosition().v;
		XMVECTOR position = XMLoadFloat3(&pos);
		float radius = mNode->GetScale().x;

		XMVECTOR toCenter = position - rayOrigin;
		float distance = XMVectorGetX(XMVector3Length(XMVector3Cross(toCenter, rayVector)));

		if (distance > radius)
		{
			printf("no intersect!\n");
			return;
		}
		else
		{
			float dt = sqrtf(radius * radius - distance * distance);
			float SurfaceDistance = XMVectorGetX(XMVector3Dot(toCenter, rayVector)) - dt;

			XMFLOAT3 surface;
			XMStoreFloat3(&surface, rayOrigin + (rayVector * SurfaceDistance));

			mNode->MoveCharacter(surface);
			mNode->PickCountry(surface);
			return;
		}
	}

	virtual void								OnMouseMove(const int& x, const int& y) override
	{
		//printf("%d %d is clicked!\n", x, y);
			
		float dx = x - prevX;
		float dy = y - prevY;

		float degree = XMConvertToRadians((dx*dx + dy*dy)*0.005f);

		Vector3 dir(dx, dy, 0);

		Vector3 axis = dir ^ Vector3(0, 0, 1);
		axis = axis.Normalized() * sinf(degree);

		//XMFLOAT4 quat = { -axis.v.x, axis.v.y, axis.v.z, cosf(degree) };
		Quaternion quat = { -axis.v.x, axis.v.y, axis.v.z, cosf(degree) };
		//mNode->MulRelativeQuaternion(quat);
		Quaternion q = mNode->GetRelativeQuaternion();
		q =  quat * q;
		mNode->SetRelativeQuaternion(q.Get());


		prevX = x;
		prevY = y;
	}

protected:

	int											prevX;
	int											prevY;
	WorldNode*									mNode;
};
;