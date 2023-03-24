#include "Unclickable.h"
#include "Engine.h"

Unclickable::Unclickable(const char* fileName, const wchar_t* name)
	: Model(fileName, name,false)
{

}

void Unclickable::Draw()
{
	Engine::mCommandList->IASetPrimitiveTopology(mTopology);
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures[mRootSignature].Get());
	Engine::mCommandList->SetPipelineState(Engine::mPSOs[mPso].Get());

	Engine::mCommandList->SetGraphicsRootDescriptorTable(2,
		Engine::mDescriptorManager->GetGpuHandle(Engine::mScenes[Engine::mCurrentScene]->mSrvIndices[mId],DescType::SRV));

	Model::Draw();
}