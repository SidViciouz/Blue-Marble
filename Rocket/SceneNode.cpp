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

	mObjFeature.diffuseAlbedo = { 0.7f, 0.9f, 0.75f };
	mObjFeature.roughness = 0.3f;
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

void SceneNode::Update()
{
	XMFLOAT4X4 parentsWorld;

	if (mParentNode == nullptr)
	{
		parentsWorld = {
			1.0f,0.0f,0.0f,0.0f,
			0.0f,1.0f,0.0f,0.0f,
			0.0f,0.0f,1.0f,0.0f,
			0.0f,0.0f,0.0f,1.0f
		};
		//copy operation
		mAccumulatedQuaternion = mRelativeQuaternion;
	}

	else
	{
		parentsWorld = mParentNode->mObjFeature.world;
		mAccumulatedQuaternion = mRelativeQuaternion * mParentNode->mAccumulatedQuaternion;
	}

	XMFLOAT3 pos = mRelativePosition.Get();
	XMMATRIX world = XMMatrixRotationQuaternion(XMLoadFloat4(&mRelativeQuaternion.Get())) *  XMMatrixTranslation(pos.x, pos.y, pos.z) *
		XMLoadFloat4x4(&parentsWorld);
	//XMMATRIX world = XMMatrixRotationQuaternion(XMLoadFloat4(&mAccumulatedQuaternion.Get())) * XMMatrixTranslation(pos.x, pos.y, pos.z);
		//* XMLoadFloat4x4(&parentsWorld);

	XMStoreFloat4x4(&mObjFeature.world, world);

	mAccumulatedPosition.Set(mObjFeature.world._41, mObjFeature.world._42, mObjFeature.world._43);

	Engine::mResourceManager->Upload(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx, &mObjFeature, sizeof(obj),
		mSceneNodeIndex * constantBufferAlignment(sizeof(obj)));

	for (auto& childNode : mChildNodes)
	{
		childNode->Update();
	}
}

bool SceneNode::IsColliding(SceneNode* counterPart, CollisionInfo& collisionInfo)
{
	if (mCollisionComponent != nullptr && counterPart->mCollisionComponent)
	{
		return mCollisionComponent->IsColliding(counterPart->mCollisionComponent.get(),collisionInfo);
	}

	return false;
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
	mRelativePosition.Set(position);
}

void SceneNode::SetRelativePosition(const float& x, const float& y, const float& z)
{
	mRelativePosition.Set(x, y, z);
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
	mAccumulatedPosition.Set(position);
	if (mParentNode == nullptr)
		mRelativePosition.Set(position);
	else
	{
		mRelativePosition.Set((mAccumulatedPosition - mParentNode->mAccumulatedPosition).Get());
	}
}

void SceneNode::SetAccumulatedPosition(const float& x, const float& y, const float& z)
{
	mAccumulatedPosition.Set(x,y,z);
	if (mParentNode == nullptr)
		mRelativePosition.Set(x,y,z);
	else
	{
		mRelativePosition.Set((mAccumulatedPosition - mParentNode->mAccumulatedPosition).Get());
	}
}

void SceneNode::AddRelativePosition(const XMFLOAT3& position)
{
	mRelativePosition.Add(position);
}

void SceneNode::AddRelativePosition(const float& x, const float& y, const float& z)
{
	mRelativePosition.Add(x, y, z);
}

void SceneNode::MulRelativeQuaternion(const Quaternion& quaternion)
{
	mRelativeQuaternion.Mul(quaternion);
}

void SceneNode::MulRelativeQuaternion(const XMFLOAT4& quaternion)
{
	mRelativeQuaternion.Mul(quaternion);
}

void SceneNode::MulRelativeQuaternion(const float& x, const float& y, const float& z, const float& w)
{
	mRelativeQuaternion.Mul(x, y, z, w);
}


Position SceneNode::GetRelativePosition()
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

Position SceneNode::GetAccumulatedPosition()
{
	return mAccumulatedPosition;
}

Quaternion SceneNode::GetAccumulatedQuaternion()
{
	return mAccumulatedQuaternion;
}