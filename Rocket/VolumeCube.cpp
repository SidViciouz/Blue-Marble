#include "VolumeCube.h"
#include "Engine.h"

VolumeCube::VolumeCube() :
	Volume()
{
	mScale = { 5.0f,5.0f,5.0f };
}

void VolumeCube::Draw()
{
	Engine::mCommandList->IASetPrimitiveTopology(mTopology);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures[mRootSignature].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs[mPso].Get());

	//Engine::mCommandList->SetGraphicsRootConstantBufferView(2, Engine::mResourceManager->GetResource(Engine::mPerlinMapIdx)->GetGPUVirtualAddress());
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mGradientsDescriptorIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(3,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPermutationDescriptorIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

	Engine::mCommandList->IASetVertexBuffers(0, 0, nullptr);
	Engine::mCommandList->IASetIndexBuffer(nullptr);
	Engine::mCommandList->DrawInstanced(36, 1, 0, 0);
}
