#include "SceneNode.h"
#include "Engine.h"

bool SceneNode::mSceneNodeNumTable[MAX_SCENE_NODE_NUM] = { false, };

SceneNode::SceneNode()
{
	mSceneNodeIndex = -1;

	for (int i = 0; i < MAX_SCENE_NODE_NUM; ++i)
	{
		if (mSceneNodeNumTable[i] == false)
		{
			mSceneNodeNumTable[i] = true;
			mSceneNodeIndex = i;
			break;
		}
	}

	mScale = { 1.0f,1.0f,1.0f };

	mObjFeature.diffuseAlbedo = { 0.3f, 0.3f, 0.3f };
	mObjFeature.roughness = 0.9f;
	mObjFeature.fresnel = { 0.1f,0.1f,0.1f };
}

void SceneNode::Draw()
{
	//drawing child nodes
	for (auto& childNode : mChildNodes)
	{
		childNode->Draw();
	}
}

void SceneNode::DrawWithoutSetting()
{
	//drawing child nodes
	for (auto& childNode : mChildNodes)
	{
		childNode->DrawWithoutSetting();
	}
}

void SceneNode::Update()
{
	if (mParentNode == nullptr)
	{
		//copy operation
		mAccumulatedQuaternion = mRelativeQuaternion;
		mAccumulatedPosition = mRelativePosition;
	}

	else
	{
		mAccumulatedQuaternion = mRelativeQuaternion * mParentNode->mAccumulatedQuaternion;
		XMVECTOR p = XMLoadFloat3(&mRelativePosition.v);
		XMMATRIX q = XMMatrixRotationQuaternion(XMLoadFloat4(&mParentNode->mAccumulatedQuaternion.Get()));
		XMFLOAT3 pq;
		XMStoreFloat3( &pq ,XMVector3TransformCoord(p, q));
		mAccumulatedPosition.v = pq;
		mAccumulatedPosition = mAccumulatedPosition + mParentNode->mAccumulatedPosition;
	}

	XMFLOAT3 pos = mAccumulatedPosition.v;
	XMMATRIX world = XMMatrixScaling(mScale.x, mScale.y, mScale.z) * XMMatrixRotationQuaternion(XMLoadFloat4(&mAccumulatedQuaternion.Get())) * XMMatrixTranslation(pos.x, pos.y, pos.z);

	XMStoreFloat4x4(&mObjFeature.world, world);

	Engine::mResourceManager->Upload(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx, &mObjFeature, sizeof(obj),
		mSceneNodeIndex * constantBufferAlignment(sizeof(obj)));

	for (auto& childNode : mChildNodes)
	{
		childNode->Update();
	}
}

void SceneNode::AddChild(shared_ptr<SceneNode> child)
{
	child->mParentNode = this;
	mChildNodes.push_back(move(child));
}

void SceneNode::RemoveChild(shared_ptr<SceneNode> child)
{

}

void SceneNode::SetRelativePosition(const XMFLOAT3& position)
{
	mRelativePosition.v = position;
}

void SceneNode::SetRelativePosition(const float& x, const float& y, const float& z)
{
	mRelativePosition.v = { x, y, z };
}

void SceneNode::SetRelativeQuaternion(const XMFLOAT4& quaternion)
{
	mRelativeQuaternion.Set(quaternion);
}

void SceneNode::SetRelativeQuaternion(const float& x, const float& y, const float& z, const float& w)
{
	mRelativeQuaternion.Set(x, y, z, w);
}

void SceneNode::SetScale(const XMFLOAT3& scale)
{
	mScale = scale;
}

void SceneNode::SetScale(const float& x, const float& y, const float& z)
{
	mScale = { x,y,z };
}

void SceneNode::SetAccumulatedPosition(const XMFLOAT3& position)
{
	mAccumulatedPosition.v = position;
	if (mParentNode == nullptr)
		mRelativePosition.v = position;
	else
	{
		XMMATRIX invQ = XMMatrixRotationQuaternion(
			XMQuaternionInverse(XMLoadFloat4(&mParentNode->mAccumulatedQuaternion.Get())));
		XMFLOAT3 pos = (mAccumulatedPosition - mParentNode->mAccumulatedPosition).v;
		XMVECTOR p = XMLoadFloat3(&pos);

		XMFLOAT3 pq;
		XMStoreFloat3(&pq, XMVector3TransformCoord(p, invQ));

		mRelativePosition.v = pq;
	}
}

void SceneNode::SetAccumulatedPosition(const float& x, const float& y, const float& z)
{
	mAccumulatedPosition.v = { x,y,z };
	if (mParentNode == nullptr)
		mRelativePosition.v = { x,y,z };
	else
	{
		//mRelativePosition.Set((mAccumulatedPosition - mParentNode->mAccumulatedPosition).Get());
		XMMATRIX invQ = XMMatrixRotationQuaternion(
			XMQuaternionInverse(XMLoadFloat4(&mParentNode->mAccumulatedQuaternion.Get())));
		XMFLOAT3 pos = (mAccumulatedPosition - mParentNode->mAccumulatedPosition).v;
		XMVECTOR p = XMLoadFloat3(&pos);

		XMFLOAT3 pq;
		XMStoreFloat3(&pq, XMVector3TransformCoord(p, invQ));

		mRelativePosition.v = pq;
	}
}

void SceneNode::AddRelativePosition(const XMFLOAT3& position)
{
	mRelativePosition += position;
}

void SceneNode::AddRelativePosition(const float& x, const float& y, const float& z)
{
	mRelativePosition.v.x += x;
	mRelativePosition.v.y += y;
	mRelativePosition.v.z += z;
}

void SceneNode::MulRelativeQuaternion(const Quaternion& quaternion)
{
	mRelativeQuaternion *= quaternion;
}

void SceneNode::MulRelativeQuaternion(const XMFLOAT4& quaternion)
{
	mRelativeQuaternion *= quaternion;
}

void SceneNode::MulRelativeQuaternion(const float& x, const float& y, const float& z, const float& w)
{
	mRelativeQuaternion.Mul(x, y, z, w);
}

void SceneNode::MulAddRelativePosition(const float& d,const XMFLOAT3& position)
{
	mRelativePosition += Vector3(d * position.x, d * position.y, d * position.z);
}


Vector3 SceneNode::GetRelativePosition()
{
	return mRelativePosition;
}

Quaternion SceneNode::GetRelativeQuaternion()
{
	return mRelativeQuaternion;
}

XMFLOAT3 SceneNode::GetScale()
{
	return mScale;
}

Vector3 SceneNode::GetAccumulatedPosition()
{
	return mAccumulatedPosition;
}

Quaternion SceneNode::GetAccumulatedQuaternion()
{
	return mAccumulatedQuaternion;
}