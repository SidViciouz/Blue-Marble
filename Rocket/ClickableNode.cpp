#include "ClickableNode.h"
#include "Engine.h"

ClickableNode::ClickableNode(string name)
	: MeshNode(name)
{
}

void ClickableNode::Draw()
{
	if (!mIsShowUp)
		return;

	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Default"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mTextureManager->GetTextureIndex("canvas"), DescType::SRV));

	Engine::mMeshManager->Draw(mMeshName);


	SceneNode::Draw();
}

void ClickableNode::Update()
{

	MeshNode::Update();
}

void ClickableNode::SetIsShowUp(bool value)
{
	mIsShowUp = value;
}

const bool& ClickableNode::GetIsShowUp() const
{
	return mIsShowUp;
}

void ClickableNode::ToggleIsShowUp()
{
	if (mIsShowUp)
		mIsShowUp = false;
	else
		mIsShowUp = true;
}