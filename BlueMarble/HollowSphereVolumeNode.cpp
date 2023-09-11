#include "HollowSphereVolumeNode.h"
#include "Engine.h"

HollowSphereVolumeNode::HollowSphereVolumeNode(float R, float r)
	: mR{ R }, mr{ r }
{

}

void HollowSphereVolumeNode::Draw()
{
	Engine::mCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["VolumeSphere"].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["VolumeSphere"].Get());

	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->GetGradientsDescriptorIdx(), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(3,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->GetPermutationDescriptorIdx(), DescType::SRV));

	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mSceneNodeIndex * Engine::mResourceManager->CalculateAlignment(sizeof(obj), 256));

	int Rr[2] = { mR,mr };
	Engine::mCommandList->SetGraphicsRoot32BitConstants(4, 2, Rr, 0);

	Engine::mCommandList->IASetVertexBuffers(0, 0, nullptr);
	Engine::mCommandList->IASetIndexBuffer(nullptr);
	Engine::mCommandList->DrawInstanced(6, 1, 0, 0);

	SceneNode::Draw();
}