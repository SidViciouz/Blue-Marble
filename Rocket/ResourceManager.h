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
	int											CreateTexture2D(int width,int height,DXGI_FORMAT format,D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	int											CreateTexture2DArray(int width, int height, int arraySize, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	int											CreateTexture3D(int width, int height, int depth, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	//int											CreateTextureCube();
	void										Copy(int src,int dst);
	int											CalculateAlignment(int value, int alignment);
	ID3D12Resource*								GetResource(int index);
	void										SwapChainPresent();

private:
	ResourceTable								mResourceTable;
	int											mNextResourceIdx = 0;
	ComPtr<IDXGISwapChain>						mSwapChain;
};
