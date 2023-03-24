#include "ResourceManager.h"
#include "Engine.h"


int ResourceManager::CreateSwapChain(int width, int height, DXGI_FORMAT format, HWND windowHandle)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = format;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0; // dx12에서는 swapchain생성시에 msaa를 지원하지 않는다.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Windowed = true;
	swapChainDesc.OutputWindow = windowHandle;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	IfError::Throw(Engine::mFactory->CreateSwapChain(Engine::mCommandQueue.Get(), &swapChainDesc, mSwapChain.GetAddressOf()),
		L"create swap chain fails!");

	mSwapChain->GetBuffer(0, IID_PPV_ARGS(mResourceTable[mNextResourceIdx++].GetAddressOf()));
	mSwapChain->GetBuffer(1, IID_PPV_ARGS(mResourceTable[mNextResourceIdx++].GetAddressOf()));

	return mNextResourceIdx - 2;
}

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

int	ResourceManager::CreateTexture1D(int width, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
{
	D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Tex1D(format,width);
	rd.Flags = flags;

	IfError::Throw(Engine::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mResourceTable[mNextResourceIdx++].GetAddressOf())
	), L"create texture1d fails!");

	return mNextResourceIdx - 1;
}

int	ResourceManager::CreateTexture2D(int width, int height, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
{
	D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);
	rd.Flags = flags;

	
	D3D12_CLEAR_VALUE clearValue;
	D3D12_RESOURCE_STATES state;

	clearValue.Format = format;
	if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
	{
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;
		state = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}
	else
	{
		for(int i=0; i<4; ++i)
			clearValue.Color[i] = 0.0f;
		if (flags == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)
			state = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		else
			state = D3D12_RESOURCE_STATE_GENERIC_READ;
	}

	IfError::Throw(Engine::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, state,
		&clearValue, IID_PPV_ARGS(mResourceTable[mNextResourceIdx++].GetAddressOf())
	), L"create texture2d fails!");

	return mNextResourceIdx - 1;
}

int	ResourceManager::CreateTexture2DArray(int width, int height, int arraySize, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
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

int	ResourceManager::CreateTexture3D(int width, int height, int depth, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags)
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

void ResourceManager::CopyUploadToTexture(int src, int dst, int width, int height, int depth, DXGI_FORMAT format, int elementByte)
{
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
	footPrint.Offset = 0;
	footPrint.Footprint.Depth = depth;
	footPrint.Footprint.Format = format;
	footPrint.Footprint.Height = height;
	footPrint.Footprint.Width = width;
	footPrint.Footprint.RowPitch = CalculateAlignment(width * elementByte, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	D3D12_TEXTURE_COPY_LOCATION tl = CD3DX12_TEXTURE_COPY_LOCATION(GetResource(dst), 0);
	D3D12_TEXTURE_COPY_LOCATION ubl = CD3DX12_TEXTURE_COPY_LOCATION(GetResource(src), footPrint);

	Engine::mCommandList->CopyTextureRegion(
		&tl,
		0, 0, 0,
		&ubl,
		nullptr
	);

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

void ResourceManager::SwapChainPresent()
{
	IfError::Throw(mSwapChain->Present(0, 0),
		L"swap chain present fails!");
}

void ResourceManager::Upload(int index, const void* data, int byteSize,int offset)
{
	BYTE* memory;
	mResourceTable[index]->Map(0, nullptr, reinterpret_cast<void**>(&memory));

	memcpy(&memory[offset], data, byteSize);

	mResourceTable[index]->Unmap(0, nullptr);
}