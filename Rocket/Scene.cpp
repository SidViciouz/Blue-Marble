#include "Scene.h"
#include "Game.h"

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
	mVertexBuffer->Copy(mAllVertices.data(), sizeof(Vertex) * mAllVertices.size(), Game::mCommandList.Get());
	mIndexBuffer->Copy(mAllIndices.data(), sizeof(uint16_t) * mAllIndices.size(), Game::mCommandList.Get());
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
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = size;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mSrvHeap.GetAddressOf())),
		L"create srv heap error!");


	auto incrementSize = Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.Texture2D.MipLevels = -1;
	viewDesc.Texture2D.MostDetailedMip = 0;
	viewDesc.Texture2D.PlaneSlice = 0;

	for (auto model = mModels->begin(); model != mModels->end(); model++)
	{
		viewDesc.Format = model->second->mTexture.mResource->GetDesc().Format;
		auto handle = mSrvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += incrementSize * model->second->mObjIndex;
		Pipeline::mDevice->CreateShaderResourceView(model->second->mTexture.mResource.Get(), &viewDesc, handle);
	}

	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	viewDesc.TextureCube.MostDetailedMip = 0;
	viewDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	for (auto world = mWorld->begin(); world != mWorld->end(); world++)
	{
		viewDesc.TextureCube.MipLevels = world->second->mTexture.mResource->GetDesc().MipLevels;
		viewDesc.Format = world->second->mTexture.mResource->GetDesc().Format;
		auto handle = mSrvHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += incrementSize * world->second->mObjIndex;
		Pipeline::mDevice->CreateShaderResourceView(world->second->mTexture.mResource.Get(), &viewDesc, handle);
	}
}

void Scene::CreateVolumeUav(int size)
{
	//create shader visible heap
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NumDescriptors = size;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mVolumeUavHeap.GetAddressOf())),
		L"create srv heap for volume error!");

	auto incrementSize = Pipeline::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (auto volume = mVolume->begin(); volume != mVolume->end(); volume++)
	{
		auto handle = mVolumeUavHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += incrementSize * volume->second->mVolumeIndex;
		Pipeline::mDevice->CreateUnorderedAccessView(volume->second->mTextureResource->mTexture.Get(), nullptr, nullptr, handle);
	}
	

	//create shader invisible heap
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NumDescriptors = size;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	IfError::Throw(Pipeline::mDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mVolumeUavHeapInvisible.GetAddressOf())),
		L"create shader invisible srv heap for volume error!");

	for (auto volume = mVolume->begin(); volume != mVolume->end(); volume++)
	{
		auto handle = mVolumeUavHeapInvisible->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += incrementSize * volume->second->mVolumeIndex;
		Pipeline::mDevice->CreateUnorderedAccessView(volume->second->mTextureResource->mTexture.Get(), nullptr, nullptr, handle);
	}
	
}
