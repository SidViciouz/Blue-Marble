#include "VolumeNode.h"
#include "Engine.h"

VolumeNode::VolumeNode(float width, float height, float depth)
	: SceneNode()
{
	mScale = { width,height,depth };
}

void VolumeNode::Draw()
{
	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Volume"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["VolumeCube"].Get());


	//Engine::mCommandList->SetGraphicsRootConstantBufferView(2, Engine::mResourceManager->GetResource(Engine::mPerlinMapIdx)->GetGPUVirtualAddress());
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mGradientsDescriptorIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(3,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mPermutationDescriptorIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

	Engine::mCommandList->IASetVertexBuffers(0, 0, nullptr);
	Engine::mCommandList->IASetIndexBuffer(nullptr);
	Engine::mCommandList->DrawInstanced(36, 1, 0, 0);

	//drawing child nodes
	for (auto& childNode : mChildNodes)
	{
		childNode->Draw();
	}
}

void VolumeNode::Update()
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
		mAccumulatedQuaternion = mRelativeQuaternion * mParentNode->GetAccumulatedQuaternion();
	}

	XMFLOAT3 pos = mRelativePosition.Get();
	XMMATRIX world = XMMatrixScaling(mScale.x, mScale.y, mScale.z) * XMMatrixRotationQuaternion(XMLoadFloat4(&mRelativeQuaternion.Get()))
		* XMMatrixTranslation(pos.x, pos.y, pos.z) * XMLoadFloat4x4(&parentsWorld);;

	XMStoreFloat4x4(&mObjFeature.world, world);

	mAccumulatedPosition.Set(mObjFeature.world._41, mObjFeature.world._42, mObjFeature.world._43);

	Engine::mResourceManager->Upload(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx, &mObjFeature, sizeof(obj),
		mSceneNodeIndex * constantBufferAlignment(sizeof(obj)));

	for (auto& childNode : mChildNodes)
	{
		childNode->Update();
	}
}