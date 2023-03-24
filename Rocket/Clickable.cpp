#include "Clickable.h"
#include "Engine.h"

Clickable::Clickable(const char* fileName, const wchar_t* name)
	: Model(fileName,name)
{

}

void Clickable::Draw()
{	
	Engine::mCommandList->IASetPrimitiveTopology(mTopology);
	
	Engine::mCommandList->SetPipelineState(Engine::mPSOs[mPso].Get());
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures[mRootSignature].Get());
	
	Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
		Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
		+ mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));
	
	//mCommandList->SetGraphicsRootDescriptorTable(2, mDescriptorManager->GetGpuHandle(mScenes[mCurrentScene]->mSrvIndices[model->first], DescType::SRV));

	
	Engine::mCommandList->SetGraphicsRootDescriptorTable(2, Engine::mDescriptorManager->GetGpuHandle(Engine::mNoiseMapDescriptorIdx,DescType::SRV));
	
	Model::Draw();
}

unique_ptr<Clickable> Clickable::Spawn(const char* fileName, const wchar_t* name,XMFLOAT3 pos)
{
	unique_ptr<Clickable> m = make_unique<Clickable>(fileName,name);
	m->SetPosition(pos);
	//현재는 sv는 생성이 안되어서 텍스처가 안보인다.

	return move(m);
}
