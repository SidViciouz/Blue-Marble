#include "IconNode.h"
#include "Engine.h"

IconNode::IconNode(string mMeshName)
	: MeshNode(mMeshName)
{
	SetScale(0.1f, 0.1f, 0.1f);
}

void IconNode::Draw()
{
	if (!mDraw)
		return;

	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["planet"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["planet"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mGradientsDescriptorIdx, DescType::SRV));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(3,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mPermutationDescriptorIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRoot32BitConstant(4, 1, 0);
	Engine::mCommandList->SetGraphicsRootDescriptorTable(5, Engine::mDescriptorManager->GetGpuHandle(
		Engine::mAllScenes[Engine::mCurrentSceneName]->mShadowMap->GetTextureSrvIdx(), DescType::SRV));

	Engine::mMeshManager->Draw(mMeshName);

	SceneNode::Draw();
}

void IconNode::DrawWithoutSetting()
{

}

void IconNode::Update()
{
	SceneNode::Update();
}

void IconNode::SetMeshName(const string& meshName)
{
	mMeshName = meshName;
}

void IconNode::SetDraw(const bool& value)
{
	mDraw = value;
}

const bool& IconNode::GetDraw() const
{
	return mDraw;
}