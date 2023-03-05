#include "Scene.h"

void Scene::Load()
{
	for (auto model = mModels->begin(); model != mModels->end(); model++)
	{
		model->second->Load();
	}
	for (auto world = mWorld->begin(); world != mWorld->end(); world++)
	{
		world->second->Load();
	}
	for (auto volume = mVolume->begin(); volume != mVolume->end(); volume++)
	{
		int data[3000] = { 1,2,3,4,5,6,7,8,9,10 };

		volume->second->mTextureResource->Copy((void*)data, 10, 10, 10, 4);
	}
}

void Scene::CreateVertexIndexBuffer()
{
	mVertexBuffer = make_unique<Buffer>(Pipeline::mDevice.Get(), sizeof(Vertex) * mAllVertices.size());
	mIndexBuffer = make_unique<Buffer>(Pipeline::mDevice.Get(), sizeof(uint16_t) * mAllIndices.size());

	//Vertex, Index 버퍼에 Model 데이터 copy
	mVertexBuffer->Copy(mAllVertices.data(), sizeof(Vertex) * mAllVertices.size(), Pipeline::mCommandList.Get());
	mIndexBuffer->Copy(mAllIndices.data(), sizeof(uint16_t) * mAllIndices.size(), Pipeline::mCommandList.Get());
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
