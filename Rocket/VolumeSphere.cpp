#include "VolumeSphere.h"
#include "Engine.h"

void VolumeSphere::Draw()
{
	Engine::mCommandList->IASetPrimitiveTopology(mTopology);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures[mRootSignature].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs[mPso].Get());

	//Engine::mCommandList->SetGraphicsRootConstantBufferView(2, Engine::mResourceManager->GetResource(Engine::mPerlinMapIdx)->GetGPUVirtualAddress());

	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mGradientsDescriptorIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(3,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mPermutationDescriptorIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

	Engine::mCommandList->IASetVertexBuffers(0, 0, nullptr);
	Engine::mCommandList->IASetIndexBuffer(nullptr);
	Engine::mCommandList->DrawInstanced(6, 1, 0, 0);
}
