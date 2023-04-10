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

	SceneNode::Draw();
}