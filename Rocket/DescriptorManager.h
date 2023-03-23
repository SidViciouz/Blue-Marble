#pragma once

#include "Util.h"

/*
* descriptor heap���� �ִ� ũ�⸦ ��Ÿ����.
*/
#define CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE 1000
#define RTV_DESCRIPTOR_HEAP_SIZE 2
#define DSV_DESCRIPTOR_HEAP_SIZE 10

/*
* descriptor�� Ÿ���� ��Ÿ����.
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
	* cbvSrvUav, rtv, dsv descriptor heap���� �����Ѵ�.
	*/
												DescriptorManager();
												DescriptorManager(const DescriptorManager& dm) = delete;
												DescriptorManager(DescriptorManager&& dm) = delete;
	DescriptorManager&							operator=(const DescriptorManager& dm) = delete;
	DescriptorManager&							operator=(DescriptorManager&& dm) = delete;

	/*
	* cbv�� �����ϰ�, descriptor heap�������� ��ġ�� ��Ÿ���� �ε����� ��ȯ�Ѵ�.
	*/
	int											CreateCbv(D3D12_GPU_VIRTUAL_ADDRESS bufferLocation,UINT sizeInBytes);
	/*
	* srv�� �����ϰ�, descriptor heap�������� ��ġ�� ��Ÿ���� �ε����� ��ȯ�Ѵ�.
	*/
	int											CreateSrv(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_SRV_DIMENSION dimension, int ArraySize = 1);
	/*
	* uav�� �����ϰ�, descriptor heap�������� ��ġ�� ��Ÿ���� �ε����� ��ȯ�Ѵ�.
	*/
	int											CreateUav(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_UAV_DIMENSION dimension, int ArraySize = 1);
	/*
	* non-visible heap�� uav�� �����ϰ�, descriptor heap�������� ��ġ�� ��Ÿ���� �ε����� ��ȯ�Ѵ�.
	*/
	int											CreateInvisibleUav(ID3D12Resource* resource, DXGI_FORMAT format, D3D12_UAV_DIMENSION dimension, int ArraySize = 1);
	/*
	* rtv�� �����ϰ�, descriptor heap�������� ��ġ�� ��Ÿ���� �ε����� ��ȯ�Ѵ�.
	*/
	int											CreateRtv(ID3D12Resource* resource);
	/*
	* dsv�� �����ϰ�, descriptor heap�������� ��ġ�� ��Ÿ���� �ε����� ��ȯ�Ѵ�.
	*/
	int											CreateDsv(ID3D12Resource* resource, D3D12_DSV_DIMENSION dimension, int ArraySize = 1,int FirstArraySlice = 0);

	/*
	* descType�� �ش��ϴ� descriptor heap�� �ش� index�� �ش��ϴ� cpu handle�� ��ȯ�Ѵ�.
	*/
	D3D12_CPU_DESCRIPTOR_HANDLE					GetCpuHandle(int index,DescType descType);
	/*
	* descType�� �ش��ϴ� descriptor heap�� �ش� index�� �ش��ϴ� gpu handle�� ��ȯ�Ѵ�.
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
	* descriptor heap�� ��밡���� ��ġ�� Ȯ���ؼ� ��ȯ�Ѵ�.
	* ��밡���� ��ġ�� ���� �ÿ�, -1�� ��ȯ�Ѵ�.
	*/
	int											GetAvailableTableIndex(DescType descType);
	/*
	* index�� �ش��ϴ� descriptor heap�� �ٽ� ��밡���� ���·� �ǵ������´�.
	*/
	void										ResetTableIndex(int index, DescType descType);

	bool										mCbvSrvUavTable[CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE] = { false, };
	bool										mNonVisibleCbvSrvUavTable[CBV_SRV_UAV_DESCRIPTOR_HEAP_SIZE] = { false, };
	bool										mRtvTable[RTV_DESCRIPTOR_HEAP_SIZE] = { false, };
	bool										mDsvTable[DSV_DESCRIPTOR_HEAP_SIZE] = { false, };
};
