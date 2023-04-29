#include "MeshNode.h"
#include "Engine.h"

MeshNode::MeshNode(string name)
	: mMeshName{ name }, SceneNode()
{

}

void MeshNode::Draw()
{
	if (!mActivated)
		return;

	//Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Default"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));
	/*
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->GetGradientsDescriptorIdx(), DescType::SRV));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(3,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->GetPermutationDescriptorIdx(), DescType::SRV));

	Engine::mCommandList->SetGraphicsRoot32BitConstant(4, 1, 0);
	Engine::mCommandList->SetGraphicsRootDescriptorTable(5, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mAllScenes[Engine::mCurrentSceneName]->mShadowMap->GetTextureSrvIdx(), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(6, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex(mTextureName), DescType::SRV));
		*/
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mTextureManager->GetTextureIndex(mTextureName), DescType::SRV));

	Engine::mMeshManager->Draw(mMeshName);

	SceneNode::Draw();
}

void MeshNode::DrawWithoutSetting()
{
	if (!mActivated)
		return;

	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));

	Engine::mMeshManager->Draw(mMeshName);

	SceneNode::DrawWithoutSetting();
}

void MeshNode::Update()
{
	/*
	* 여기에 추가로 필요한 것들을 작성한다.
	*/
	if (!mActivated)
		return;

	SceneNode::Update();
}

const string& MeshNode::GetMeshName() const
{
	return mMeshName;
}

void MeshNode::SetActivated(const bool& value)
{
	mActivated = value;
}

const bool& MeshNode::GetActivated() const
{
	return mActivated;
}

void MeshNode::SetTextureName(const string& name)
{
	mTextureName = name;
}