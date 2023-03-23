#include "ResourceManager.h"
#include "Engine.h"


int	ResourceManager::CreateUploadBuffer(int byteSize)
{
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.VisibleNodeMask = 0;
	heapProperties.CreationNodeMask = 0;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = byteSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//upload힙을 생성할 때 resource state는 generic read여야함. (documentation참고)
	IfError::Throw(Engine::mDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(mResourceTable[mNextResourceIdx++].GetAddressOf())),
		L"create upload buffer fail!");

	return mNextResourceIdx - 1;
}

int	ResourceManager::CreateDefaultBuffer(int byteSize)
{
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.VisibleNodeMask = 0;
	heapProperties.CreationNodeMask = 0;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = byteSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	IfError::Throw(Engine::mDevice->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(mResourceTable[mNextResourceIdx++].GetAddressOf())),
		L"create default buffer fail!");

	return mNextResourceIdx - 1;
}

int	ResourceManager::CreateTexture2D(int width, int height, int elementByte, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
{
	D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);
	rd.Flags = flags;

	IfError::Throw(Engine::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mResourceTable[mNextResourceIdx++].GetAddressOf())
	), L"create texture2d fails!");

	return mNextResourceIdx - 1;
}

int	ResourceManager::CreateTexture2DArray(int width, int height, int arraySize, int elementByte, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
{
	D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, arraySize);
	rd.Flags = flags;

	IfError::Throw(Engine::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mResourceTable[mNextResourceIdx++].GetAddressOf())
	), L"create texture 2d array fails!");

	return mNextResourceIdx - 1;
}

int	ResourceManager::CreateTexture3D(int width, int height, int depth, int elementByte, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
{
	D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Tex3D(format, width, height, depth);
	rd.Flags = flags;

	IfError::Throw(Engine::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mResourceTable[mNextResourceIdx++].GetAddressOf())
	), L"create texture 3d fails!");

	return mNextResourceIdx - 1;
}

/*
int	ResourceManager::CreateTextureCube()
{

}
*/

void ResourceManager::Copy(int src, int dst)
{

}

int ResourceManager::CalculateAlignment(int value, int alignment)
{
	//alignment가 2의 거듭제곱이여야한다.
	return (value + alignment - 1) & ~(alignment - 1);
}

ID3D12Resource* ResourceManager::GetResource(int index)
{
	if (index < mNextResourceIdx)
		return mResourceTable[index].Get();

	return nullptr;
}