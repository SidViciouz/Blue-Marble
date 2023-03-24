#include "Scene.h"
#include "Engine.h"

Scene::Scene() :
	mSpawnSystem(make_unique<SpawnSystem>())
{

}

void Scene::Load()
{
	for (auto volume = mVolume->begin(); volume != mVolume->end(); volume++)
	{
		volume->second->mTextureResource->Create(100, 100, 100, 4);
	}
}

void Scene::Update()
{
	for (auto it = mModels->begin(); it != mModels->end(); it++)
	{
		if (it->second->mDirty)
		{
			it->second->Update();
		}
	}

	for (auto it = mWorld->begin(); it != mWorld->end(); it++)
	{
		if (it->second->mDirty)
		{
			it->second->Update();
		}
	}

	for (auto it = mVolume->begin(); it != mVolume->end(); it++)
	{
		if (it->second->mDirty)
		{
			it->second->Update();
		}
	}
}

void Scene::CreateModelSrv(int size)
{
	for (auto model = mModels->begin(); model != mModels->end(); model++)
	{
		int index = Engine::mDescriptorManager->CreateSrv(model->second->mTexture.mResource.Get(),
			model->second->mTexture.mResource->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D);
		
		mSrvIndices[model->first] = index;
	}


	for (auto world = mWorld->begin(); world != mWorld->end(); world++)
	{
		int index = Engine::mDescriptorManager->CreateSrv(world->second->mTexture.mResource.Get(),
			world->second->mTexture.mResource->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURECUBE);
		mSrvIndices[world->first] = index;
	}
}

void Scene::CreateVolumeUav(int size)
{
	for (auto volume = mVolume->begin(); volume != mVolume->end(); volume++)
	{
		int index = Engine::mDescriptorManager->CreateUav(volume->second->mTextureResource->mTexture.Get(),
			DXGI_FORMAT_R32_FLOAT, D3D12_UAV_DIMENSION_TEXTURE3D);
		mSrvIndices[volume->first] = index;

		index = Engine::mDescriptorManager->CreateInvisibleUav(volume->second->mTextureResource->mTexture.Get(),
			DXGI_FORMAT_R32_FLOAT, D3D12_UAV_DIMENSION_TEXTURE3D);
		mInvisibleUavIndices[volume->first] = index;
	}
}

void Scene::Spawn()
{
	mSpawnSystem->Spawn(mModels,mWorld,mVolume);
}

void Scene::Destroy()
{
	mSpawnSystem->Destroy(mModels, mWorld, mVolume);
}

bool Scene::IsDestroyQueueEmpty() const
{
	return mSpawnSystem->IsDestroyQueueEmpty();
}

void Scene::Draw()
{
	//particle density update
	Engine::mCommandList->SetPipelineState(Engine::mPSOs["Particle"].Get());
	Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Particle"].Get());
	Engine::mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	Engine::mCommandList->IASetVertexBuffers(0, 1, Engine::mParticleField->GetVertexBufferView());
	for (auto volume = Engine::mScenes[Engine::mCurrentScene]->mVolume->begin(); volume != Engine::mScenes[Engine::mCurrentScene]->mVolume->end(); volume++)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = Engine::mDescriptorManager->GetGpuHandle(Engine::mScenes[Engine::mCurrentScene]->mSrvIndices[volume->first], DescType::UAV);
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = Engine::mDescriptorManager->GetCpuHandle(Engine::mScenes[Engine::mCurrentScene]->mInvisibleUavIndices[volume->first], DescType::iUAV);
		UINT color[4] = { 0,0,0,0 };
		Engine::mCommandList->ClearUnorderedAccessViewUint(gpuHandle, cpuHandle, volume->second->mTextureResource->mTexture.Get(), color, 0, nullptr);
		Engine::mCommandList->SetGraphicsRootDescriptorTable(0, gpuHandle);
		Engine::mCommandList->DrawInstanced(Engine::mParticleField->NumParticle(), 1, 0, 0);
	}
	//

	for (auto world = mWorld->begin(); world != mWorld->end(); world++)
	{
		world->second->Draw();
	}

	//선택된 물체에 노란색 테두리 렌더링
	if (mIsModelSelected == true)
	{
		Engine::mCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Engine::mCommandList->SetGraphicsRootSignature(Engine::mRootSignatures["Default"].Get());
		Engine::mCommandList->SetPipelineState(Engine::mPSOs["Selected"].Get());
		Engine::mCommandList->SetGraphicsRootConstantBufferView(0,
			Engine::mResourceManager->GetResource(Engine::mFrames[Engine::mCurrentFrame]->mObjConstantBufferIdx)->GetGPUVirtualAddress()
			+ mSelectedModel->mObjIndex * BufferInterface::ConstantBufferByteSize(sizeof(obj)));

		mSelectedModel->Model::Draw();
	}

	for (auto model = mModels->begin(); model != mModels->end(); model++)
	{
		model->second->Draw();
	}

	for (auto volume = mVolume->begin(); volume != mVolume->end(); volume++)
	{
		volume->second->Draw();
	}
}