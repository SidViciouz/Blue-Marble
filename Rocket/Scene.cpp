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
