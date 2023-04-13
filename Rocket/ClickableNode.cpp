#include "ClickableNode.h"
#include "Engine.h"

ClickableNode::ClickableNode(string name)
	: MeshNode(name)
{
}

void ClickableNode::Draw()
{
	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Default"].Get());
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mGradientsDescriptorIdx, DescType::SRV));

	Engine::mMeshManager->Draw(mMeshName);


	SceneNode::Draw();
}

void ClickableNode::Update()
{

	MeshNode::Update();
}