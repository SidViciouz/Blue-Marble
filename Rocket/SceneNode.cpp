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

void SceneNode::Update(const XMFLOAT4X4& parentsWorld)
{
	if (mDirty == true)
	{
		XMFLOAT3 pos = mRelativePosition.Get();

		XMMATRIX world = XMMatrixRotationQuaternion(XMLoadFloat4(&mRelativeQuaternion.Get())) * XMMatrixTranslation(pos.x,pos.y,pos.z)
			* XMLoadFloat4x4(&parentsWorld);

		XMStoreFloat4x4(&mObjFeature.world, world);

		mDirty = false;
	}

	Engine::mResourceManager->Upload(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx, &mObjFeature, sizeof(obj),
		mSceneNodeIndex * constantBufferAlignment(sizeof(obj)));

	for (auto& childNode : mChildNodes)
	{
		childNode->Update(mObjFeature.world);
	}
}

void SceneNode::AddChild(unique_ptr<SceneNode> child)
{
	mChildNodes.push_back(move(child));
}

void SceneNode::RemoveChild(unique_ptr<SceneNode> child)
{

}