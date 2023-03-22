#pragma once

#include "Util.h"
#include "Engine.h"

#define CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE 1000
#define RTV_DESCRIPTOR_HEAP_SIZE 2
#define DSV_DESCRIPTOR_HEAP_SIZE 10

enum class DescType
{
	CBV,
	SRV,
	UAV,
	RTV,
	DSV
};

class DescriptorManager
{
public:
												DescriptorManager();
												DescriptorManager(const DescriptorManager& dm) = delete;
												DescriptorManager(DescriptorManager&& dm) = delete;
	DescriptorManager&							operator=(const DescriptorManager& dm) = delete;
	DescriptorManager&							operator=(DescriptorManager&& dm) = delete;

	int											CreateCbv(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation,UINT sizeInBytes);
	int											CreateSrv(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_SRV_DIMENSION dimension, int ArraySize = 1);
	int											CreateUav(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_UAV_DIMENSION dimension, int ArraySize = 1);
	int											CreateRtv(ID3D12Resource* resource);
	int											CreateDsv(ID3D12Resource* resource, D3D12_DSV_DIMENSION dimension, int ArraySize = 1,int FirstArraySlice = 0);


private:
	ComPtr<ID3D12DescriptorHeap>				mCbvSrvUavHeap;
	ComPtr<ID3D12DescriptorHeap>				mNonVisibleCbvSrvUavHeap;
	ComPtr<ID3D12DescriptorHeap>				mRtvHeap;
	ComPtr<ID3D12DescriptorHeap>				mDsvHeap;
	UINT										mCbvSrvUavIncrementSize;
	UINT										mRtvIncrementSize;
	UINT										mDsvIncrementSize;
	D3D12_CPU_DESCRIPTOR_HANDLE					mCbvSrvUavCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE					mCbvSrvUavGpuHandleStart;
	D3D12_CPU_DESCRIPTOR_HANDLE					mRtvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE					mRtvGpuHandleStart;
	D3D12_CPU_DESCRIPTOR_HANDLE					mDsvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE					mDsvGpuHandleStart;
	
	D3D12_CPU_DESCRIPTOR_HANDLE					GetCpuHandle(int index,DescType descType);
	D3D12_GPU_DESCRIPTOR_HANDLE					GetGpuHandle(int index, DescType descType);

	int											GetAvailableTableIndex(DescType descType);
	void										ResetTableIndex(int index, DescType descType);

	bool										mCbvSrvUavTable[CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE] = { false, };
	bool										mRtvTable[RTV_DESCRIPTOR_HEAP_SIZE] = { false, };
	bool										mDsvTable[DSV_DESCRIPTOR_HEAP_SIZE] = { false, };
};
