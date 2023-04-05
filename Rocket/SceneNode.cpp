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