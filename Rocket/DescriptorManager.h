#pragma once

#include "Util.h"
#include "Engine.h"

#define TOTAL_DESCRIPTOR_HEAP_SIZE 1000

class DescriptorManager
{
public:
												DescriptorManager();
												DescriptorManager(const DescriptorManager& dm) = delete;
												DescriptorManager(DescriptorManager&& dm) = delete;
	DescriptorManager&							operator=(const DescriptorManager& dm) = delete;
	DescriptorManager&							operator=(DescriptorManager&& dm) = delete;

	D3D12_CPU_DESCRIPTOR_HANDLE					GetCPUHandle(int handle);
	D3D12_GPU_DESCRIPTOR_HANDLE					GetGPUHandle(int handle);
	

private:
	ComPtr<ID3D12DescriptorHeap>				mCbvSrvUavHeap;
	ComPtr<ID3D12DescriptorHeap>				mRtvHeap;
	ComPtr<ID3D12DescriptorHeap>				mDsvHeap;
	int											mCbvSrvUavIncrementSize;
	int											mRtvIncrementSize;
	int											mDsvIncrementSize;
	bool										mDescriptorTable[TOTAL_DESCRIPTOR_HEAP_SIZE] = { false, };
};
