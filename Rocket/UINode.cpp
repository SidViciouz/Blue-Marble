#include "UINode.h"
#include "Engine.h"

UINode::UINode(string name)
	: MeshNode(name)
{
}

void UINode::Draw()
{
	if (!mIsShowUp)
		return;

	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Default"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));
	if (!mTextureName.empty())
	{
		Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
			Engine::mDescriptorManager->GetGpuHandle(Engine::mTextureManager->GetTextureIndex(mTextureName), DescType::SRV));
	}
	Engine::mMeshManager->Draw(mMeshName);


	SceneNode::Draw();
}

void UINode::Update()
{

	MeshNode::Update();
}

void UINode::SetIsShowUp(bool value)
{
	mIsShowUp = value;
}

const bool& UINode::GetIsShowUp() const
{
	return mIsShowUp;
}

void UINode::ToggleIsShowUp()
{
	if (mIsShowUp)
		mIsShowUp = false;
	else
		mIsShowUp = true;
}