#include "Clickable.h"
#include "Engine.h"

Clickable::Clickable(const char* fileName, const wchar_t* name)
	: Model(fileName,name)
{

}

void Clickable::Draw()
{
	Engine::mCommandList->IASetPrimitiveTopology(mTopology);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures[mRootSignature].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs[mPso].Get());

	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

	//Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
	//	Engine::mDescriptorManager->GetGpuHandle(Engine::mScenes[Engine::mCurrentScene]->mSrvIndices[mId], DescType::SRV));
	// 
	//Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(Engine::mNoiseMapDescriptorIdx, DescType::SRV));
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mGradientsDescriptorIdx, DescType::SRV));

	Engine::mCommandList->SetGraphicsRootDescriptorTable(3,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mPerlinMap->mPermutationDescriptorIdx, DescType::SRV));

	Model::Draw();
}

unique_ptr<Clickable> Clickable::Spawn(const char* fileName, const wchar_t* name,XMFLOAT3 pos)
{
	unique_ptr<Clickable> m = make_unique<Clickable>(fileName,name);
	m->SetPosition(pos);
	//현재는 sv는 생성이 안되어서 텍스처가 안보인다.

	return move(m);
}
