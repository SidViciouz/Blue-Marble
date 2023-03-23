#pragma once

#include "Util.h"

/*
* descriptor heap들의 최대 크기를 나타낸다.
*/
#define CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE 1000
#define RTV_DESCRIPTOR_HEAP_SIZE 2
#define DSV_DESCRIPTOR_HEAP_SIZE 10

/*
* descriptor의 타입을 나타낸다.
*/
enum class DescType
{
	CBV,
	SRV,
	UAV,
	RTV,
	DSV,
	iUAV
};

class DescriptorManager
{
public:
	/*
	* cbvSrvUav, rtv, dsv descriptor heap들을 생성한다.
	*/
												DescriptorManager();
												DescriptorManager(const DescriptorManager& dm) = delete;
												DescriptorManager(DescriptorManager&& dm) = delete;
	DescriptorManager&							operator=(const DescriptorManager& dm) = delete;
	DescriptorManager&							operator=(DescriptorManager&& dm) = delete;

	/*
	* cbv를 생성하고, descriptor heap내에서의 위치를 나타내는 인덱스를 반환한다.
	*/
	int											CreateCbv(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation,UINT sizeInBytes);
	/*
	* srv를 생성하고, descriptor heap내에서의 위치를 나타내는 인덱스를 반환한다.
	*/
	int											CreateSrv(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_SRV_DIMENSION dimension, int ArraySize = 1);
	/*
	* uav를 생성하고, descriptor heap내에서의 위치를 나타내는 인덱스를 반환한다.
	*/
	int											CreateUav(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_UAV_DIMENSION dimension, int ArraySize = 1);
	/*
	* non-visible heap에 uav를 생성하고, descriptor heap내에서의 위치를 나타내는 인덱스를 반환한다.
	*/
	int											CreateInvisibleUav(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_UAV_DIMENSION dimension, int ArraySize = 1);
	/*
	* rtv를 생성하고, descriptor heap내에서의 위치를 나타내는 인덱스를 반환한다.
	*/
	int											CreateRtv(ID3D12Resource* resource);
	/*
	* dsv를 생성하고, descriptor heap내에서의 위치를 나타내는 인덱스를 반환한다.
	*/
	int											CreateDsv(ID3D12Resource* resource, D3D12_DSV_DIMENSION dimension, int ArraySize = 1,int FirstArraySlice = 0);

	/*
	* descType에 해당하는 descriptor heap의 해당 index에 해당하는 cpu handle을 반환한다.
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE					GetCpuHandle(int index,DescType descType);
	/*
	* descType에 해당하는 descriptor heap의 해당 index에 해당하는 gpu handle을 반환한다.
	*/
	D3D12_GPU_DESCRIPTOR_HANDLE					GetGpuHandle(int index, DescType descType);

	ID3D12DescriptorHeap**						GetHeapAddress(DescType descType);

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
	D3D12_CPU_DESCRIPTOR_HANDLE					mNonVisibleCbvSrvUavCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE					mNonVisibleCbvSrvUavGpuHandleStart;
	D3D12_CPU_DESCRIPTOR_HANDLE					mRtvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE					mRtvGpuHandleStart;
	D3D12_CPU_DESCRIPTOR_HANDLE					mDsvCpuHandleStart;
	D3D12_GPU_DESCRIPTOR_HANDLE					mDsvGpuHandleStart;
	
	/*
	* descriptor heap의 사용가능한 위치를 확인해서 반환한다.
	* 사용가능한 위치가 없을 시에, -1을 반환한다.
	*/
	int											GetAvailableTableIndex(DescType descType);
	/*
	* index에 해당하는 descriptor heap을 다시 사용가능한 상태로 되돌려놓는다.
	*/
	void										ResetTableIndex(int index, DescType descType);

	bool										mCbvSrvUavTable[CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE] = { false, };
	bool										mNonVisibleCbvSrvUavTable[CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE] = { false, };
	bool										mRtvTable[RTV_DESCRIPTOR_HEAP_SIZE] = { false, };
	bool										mDsvTable[DSV_DESCRIPTOR_HEAP_SIZE] = { false, };
};
