#include "SceneNode.h"

void SceneNode::Draw()
{
	for (int meshIndex : mMeshIndices)
	{
		//drawMesh(meshIndex);
	}
	
	for (int volumeIndex : mVolumeIndices)
	{
		//drawVolume(volumeIndex);
	}

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

	for (auto& childNode : mChildNodes)
	{
		childNode->Update(mObjFeature.world);
	}
}