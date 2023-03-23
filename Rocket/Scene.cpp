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
	//create shader visible heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = size;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	IfError::Throw(Engine::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mVolumeUavHeap.GetAddressOf())),
		L"create srv heap for volume error!");

	auto incrementSize = Engine::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (auto volume = mVolume->begin(); volume != mVolume->end(); volume++)
	{
		auto handle = mVolumeUavHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += incrementSize * volume->second->mVolumeIndex;
		Engine::mDevice->CreateUnorderedAccessView(volume->second->mTextureResource->mTexture.Get(), nullptr, nullptr, handle);
	}
	

	//create shader invisible heap
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NumDescriptors = size;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	IfError::Throw(Engine::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mVolumeUavHeapInvisible.GetAddressOf())),
		L"create shader invisible srv heap for volume error!");

	for (auto volume = mVolume->begin(); volume != mVolume->end(); volume++)
	{
		auto handle = mVolumeUavHeapInvisible->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += incrementSize * volume->second->mVolumeIndex;
		Engine::mDevice->CreateUnorderedAccessView(volume->second->mTextureResource->mTexture.Get(), nullptr, nullptr, handle);
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
