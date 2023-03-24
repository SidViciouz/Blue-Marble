#pragma once

#include "Util.h"

#define MAX_RESOURCE_SIZE 1000

using ResourceTable = array<ComPtr<ID3D12Resource>, MAX_RESOURCE_SIZE>;

class ResourceManager
{
public:
	int											CreateSwapChain(int width, int height, DXGI_FORMAT format, HWND windowHandle);
	int											CreateUploadBuffer(int byteSize);
	int											CreateDefaultBuffer(int byteSize);
	int											CreateTexture1D(int width, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	int											CreateTexture2D(int width,int height,DXGI_FORMAT format,D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	int											CreateTexture2DArray(int width, int height, int arraySize, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	int											CreateTexture3D(int width, int height, int depth, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	/*
	* 한 resource에서 다른 resource로 data를 copy한다. (GPU->GPU)
	*/
	void										Copy(int src,int dst,int width,int height,int depth,DXGI_FORMAT format, int elementByte);
	int											CalculateAlignment(int value, int alignment);
	ID3D12Resource*								GetResource(int index);
	void										SwapChainPresent();
	void										Upload(int index, const void* data, int byteSize,int offset);

private:
	ResourceTable								mResourceTable;
	int											mNextResourceIdx = 0;
	ComPtr<IDXGISwapChain>						mSwapChain;
};


static UINT constantBufferAlignment(UINT byteSize)
{
	return (byteSize + 255) & ~255;
}

