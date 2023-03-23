#include "DescriptorManager.h"
#include "Engine.h"

DescriptorManager::DescriptorManager()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavDesc = {};
	cbvSrvUavDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvSrvUavDesc.NodeMask = 0;
	cbvSrvUavDesc.NumDescriptors = CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE;
	cbvSrvUavDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	IfError::Throw(Engine::mDevice->CreateDescriptorHeap(&cbvSrvUavDesc,IID_PPV_ARGS(mCbvSrvUavHeap.GetAddressOf())),
		L"Creating CbvSrvUav heap fails!");

	D3D12_DESCRIPTOR_HEAP_DESC nonVisibleDesc = {};
	nonVisibleDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	nonVisibleDesc.NodeMask = 0;
	nonVisibleDesc.NumDescriptors = CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE;
	nonVisibleDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	IfError::Throw(Engine::mDevice->CreateDescriptorHeap(&nonVisibleDesc, IID_PPV_ARGS(mNonVisibleCbvSrvUavHeap.GetAddressOf())),
		L"Creating non-visible CbvSrvUav heap fails!");

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeap = {};
	rtvHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeap.NodeMask = 0;
	rtvHeap.NumDescriptors = RTV_DESCRIPTOR_HEAP_SIZE;
	rtvHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	IfError::Throw(Engine::mDevice->CreateDescriptorHeap(&rtvHeap, IID_PPV_ARGS(mRtvHeap.GetAddressOf())),
		L"Creating Rtv heap fails!");

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeap = {};
	dsvHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeap.NodeMask = 0;
	dsvHeap.NumDescriptors = DSV_DESCRIPTOR_HEAP_SIZE;
	dsvHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	IfError::Throw(Engine::mDevice->CreateDescriptorHeap(&dsvHeap, IID_PPV_ARGS(mDsvHeap.GetAddressOf())),
		L"Creating Dsv heap fails!");


	mCbvSrvUavIncrementSize = Engine::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	mRtvIncrementSize = Engine::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvIncrementSize = Engine::mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	mCbvSrvUavCpuHandleStart = mCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	mCbvSrvUavGpuHandleStart = mCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	mNonVisibleCbvSrvUavCpuHandleStart = mNonVisibleCbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart();
	mNonVisibleCbvSrvUavGpuHandleStart = mNonVisibleCbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
	mRtvCpuHandleStart = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
	mRtvGpuHandleStart = mRtvHeap->GetGPUDescriptorHandleForHeapStart();
	mDsvCpuHandleStart = mDsvHeap->GetCPUDescriptorHandleForHeapStart();
	mDsvGpuHandleStart = mDsvHeap->GetGPUDescriptorHandleForHeapStart();
}

int	DescriptorManager::CreateCbv(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation, UINT sizeInBytes)
{
	int index = GetAvailableTableIndex(DescType::CBV);
	if (index == -1)
		return -1;
	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = GetCpuHandle(index, DescType::CBV);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = bufferLocation;
	cbvDesc.SizeInBytes = sizeInBytes;

	Engine::mDevice->CreateConstantBufferView(&cbvDesc, cbvHandle);

	return index;
}

int	DescriptorManager::CreateSrv(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_SRV_DIMENSION dimension, int ArraySize)
{
	int index = GetAvailableTableIndex(DescType::SRV);
	if (index == -1)
		return -1;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = GetCpuHandle(index,DescType::SRV);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = format;
	srvDesc.ViewDimension = dimension;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (dimension == D3D12_SRV_DIMENSION_TEXTURE2D)
	{
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}
	else if (dimension == D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
	{
		srvDesc.Texture2DArray.ArraySize = ArraySize;
		srvDesc.Texture2DArray.MipLevels = -1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.PlaneSlice = 0;
		srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
	}
	else if (dimension == D3D12_SRV_DIMENSION_TEXTURE3D)
	{
		srvDesc.Texture3D.MipLevels = -1;
		srvDesc.Texture3D.MostDetailedMip = 0;
		srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
	}
	else if (dimension == D3D12_SRV_DIMENSION_TEXTURECUBE)
	{
		srvDesc.TextureCube.MipLevels = -1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	}
	else
	{
		ResetTableIndex(index, DescType::SRV);
		return -1;
	}

	Engine::mDevice->CreateShaderResourceView(resource, &srvDesc, srvHandle);

	return index;
}

int	DescriptorManager::CreateUav(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_UAV_DIMENSION dimension, int ArraySize)
{
	int index = GetAvailableTableIndex(DescType::UAV);
	if (index == -1)
		return -1;
	D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = GetCpuHandle(index, DescType::UAV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = format;
	uavDesc.ViewDimension = dimension;

	if (dimension == D3D12_UAV_DIMENSION_TEXTURE2D)
	{
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Texture2D.PlaneSlice = 0;
		
	}
	else if (dimension == D3D12_UAV_DIMENSION_TEXTURE2DARRAY)
	{
		uavDesc.Texture2DArray.ArraySize = ArraySize;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.PlaneSlice = 0;
	}
	else if (dimension == D3D12_UAV_DIMENSION_TEXTURE3D)
	{
		uavDesc.Texture3D.FirstWSlice = 0;
		uavDesc.Texture3D.MipSlice = 0;
		uavDesc.Texture3D.WSize = -1;
	}
	else
	{
		ResetTableIndex(index, DescType::UAV);
		return -1;
	}

	Engine::mDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, uavHandle);

	return index;
}

int DescriptorManager::CreateInvisibleUav(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_UAV_DIMENSION dimension, int ArraySize)
{
	int index = GetAvailableTableIndex(DescType::iUAV);
	if (index == -1)
		return -1;
	D3D12_CPU_DESCRIPTOR_HANDLE uavHandle = GetCpuHandle(index, DescType::iUAV);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = format;
	uavDesc.ViewDimension = dimension;

	if (dimension == D3D12_UAV_DIMENSION_TEXTURE2D)
	{
		uavDesc.Texture2D.MipSlice = 0;
		uavDesc.Texture2D.PlaneSlice = 0;

	}
	else if (dimension == D3D12_UAV_DIMENSION_TEXTURE2DARRAY)
	{
		uavDesc.Texture2DArray.ArraySize = ArraySize;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.PlaneSlice = 0;
	}
	else if (dimension == D3D12_UAV_DIMENSION_TEXTURE3D)
	{
		uavDesc.Texture3D.FirstWSlice = 0;
		uavDesc.Texture3D.MipSlice = 0;
		uavDesc.Texture3D.WSize = -1;
	}
	else
	{
		ResetTableIndex(index, DescType::iUAV);
		return -1;
	}

	Engine::mDevice->CreateUnorderedAccessView(resource, nullptr, &uavDesc, uavHandle);

	return index;
}

int	DescriptorManager::CreateRtv(ID3D12Resource* resource)
{
	int index = GetAvailableTableIndex(DescType::RTV);
	if (index == -1)
		return -1;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetCpuHandle(index, DescType::RTV);

	Engine::mDevice->CreateRenderTargetView(resource,nullptr,rtvHandle);

	return index;
}

int	DescriptorManager::CreateDsv(ID3D12Resource* resource,D3D12_DSV_DIMENSION dimension, int ArraySize,int FirstArraySlice)
{
	int index = GetAvailableTableIndex(DescType::DSV);
	if (index == -1)
		return -1;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetCpuHandle(index, DescType::DSV);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = dimension;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	if (dimension == D3D12_DSV_DIMENSION_TEXTURE2D)
	{
		dsvDesc.Texture2D.MipSlice = 0;
	}
	else if (dimension == D3D12_DSV_DIMENSION_TEXTURE2DARRAY)
	{
		dsvDesc.Texture2DArray.ArraySize = ArraySize;
		dsvDesc.Texture2DArray.FirstArraySlice = FirstArraySlice;
		dsvDesc.Texture2DArray.MipSlice = 0;
	}
	else
	{
		ResetTableIndex(index, DescType::DSV);
		return -1;
	}

	Engine::mDevice->CreateDepthStencilView(resource, &dsvDesc, dsvHandle);
	return index;
}

D3D12_CPU_DESCRIPTOR_HANDLE	DescriptorManager::GetCpuHandle(int index, DescType descType)
{
	if (descType == DescType::CBV || descType == DescType::SRV || descType == DescType::UAV)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mCbvSrvUavCpuHandleStart;
		handle.ptr += index * mCbvSrvUavIncrementSize;

		return handle;
	}
	else if (descType == DescType::RTV)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mRtvCpuHandleStart;
		handle.ptr += index * mRtvIncrementSize;

		return handle;
	}
	else if (descType == DescType::DSV)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mDsvCpuHandleStart;
		handle.ptr += index * mDsvIncrementSize;

		return handle;
	}
	else if (descType == DescType::iUAV)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = mNonVisibleCbvSrvUavCpuHandleStart;
		handle.ptr += index * mCbvSrvUavIncrementSize;

		return handle;
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE	DescriptorManager::GetGpuHandle(int index, DescType descType)
{
	if (descType == DescType::CBV || descType == DescType::SRV || descType == DescType::UAV)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = mCbvSrvUavGpuHandleStart;
		handle.ptr += index * mCbvSrvUavIncrementSize;

		return handle;
	}
	else if (descType == DescType::RTV)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = mRtvGpuHandleStart;
		handle.ptr += index * mRtvIncrementSize;

		return handle;
	}
	else if (descType == DescType::DSV)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = mDsvGpuHandleStart;
		handle.ptr += index * mDsvIncrementSize;

		return handle;
	}
	else if (descType == DescType::iUAV)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = mNonVisibleCbvSrvUavGpuHandleStart;
		handle.ptr += index * mCbvSrvUavIncrementSize;

		return handle;
	}
}

ID3D12DescriptorHeap** DescriptorManager::GetHeapAddress(DescType descType)
{
	if (descType == DescType::CBV || descType == DescType::SRV || descType == DescType::UAV)
	{
		return mCbvSrvUavHeap.GetAddressOf();
	}
	else if (descType == DescType::RTV)
	{
		return mRtvHeap.GetAddressOf();
	}
	else if (descType == DescType::DSV)
	{
		return mDsvHeap.GetAddressOf();
	}

	return nullptr;
}

int DescriptorManager::GetAvailableTableIndex(DescType descType)
{
	if (descType == DescType::CBV || descType == DescType::SRV || descType == DescType::UAV)
	{
		for (int i = 0; i < CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE; ++i)
		{
			if (mCbvSrvUavTable[i] == false)
			{
				mCbvSrvUavTable[i] = true;
				return i;
			}
		}
	}
	else if (descType == DescType::RTV)
	{
		for (int i = 0; i < RTV_DESCRIPTOR_HEAP_SIZE; ++i)
		{
			if (mRtvTable[i] == false)
			{
				mRtvTable[i] = true;
				return i;
			}
		}
	}
	else if (descType == DescType::DSV)
	{
		for (int i = 0; i < DSV_DESCRIPTOR_HEAP_SIZE; ++i)
		{
			if (mDsvTable[i] == false)
			{
				mDsvTable[i] = true;
				return i;
			}
		}
	}
	else if (descType == DescType::iUAV)
	{
		for (int i = 0; i < CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE; ++i)
		{
			if (mNonVisibleCbvSrvUavTable[i] == false)
			{
				mNonVisibleCbvSrvUavTable[i] = true;
				return i;
			}
		}
	}
	
	return -1;
}

void DescriptorManager::ResetTableIndex(int index, DescType descType)
{
	if (descType == DescType::CBV || descType == DescType::SRV || descType == DescType::UAV)
	{
		if (index < CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE)
		{
			mCbvSrvUavTable[index] = false;
		}
	}
	else if (descType == DescType::RTV)
	{
		if (index < RTV_DESCRIPTOR_HEAP_SIZE)
		{
			mRtvTable[index] = false;
		}
	}
	else if (descType == DescType::DSV)
	{
		if (index < DSV_DESCRIPTOR_HEAP_SIZE)
		{
			mDsvTable[index] = false;
		}
	}
	else if (descType == DescType::iUAV)
	{
		if (index < CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE)
		{
			mNonVisibleCbvSrvUavTable[index] = false;
		}
	}
}