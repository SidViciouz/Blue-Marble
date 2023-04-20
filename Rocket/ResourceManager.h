#pragma once

#include "Util.h"

#define MAX_RESOURCE_SIZE 1000

using ResourceTable = array<ComPtr<ID3D12Resource>, MAX_RESOURCE_SIZE>;

class ResourceManager
{
public:
	/*
	* swapchain을 생성한다.
	*/
	int											CreateSwapChain(int width, int height, DXGI_FORMAT format, HWND windowHandle);
	/*
	* upload buffer를 정해진 크기로 생성한다.
	*/
	int											CreateUploadBuffer(int byteSize);
	/*
	* default buffer를 정해진 크기로 생성한다.
	*/
	int											CreateDefaultBuffer(int byteSize);
	/*
	* 1차원 텍스처를 생성한다.
	*/
	int											CreateTexture1D(int width, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	/*
	* 2차원 텍스처를 생성한다.
	*/
	int											CreateTexture2D(int width,int height,DXGI_FORMAT format,D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	/*
	* 2차원 텍스처의 배열을 생성한다.
	*/
	int											CreateTexture2DArray(int width, int height, int arraySize, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	/*
	* 3차원 텍스처를 생성한다.
	*/
	int											CreateTexture3D(int width, int height, int depth, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	/*
	* 한 resource에서 다른 resource로 texture data를 copy한다. (GPU->GPU)
	*/
	void										CopyUploadToTexture(int src,int dst,int width,int height,int depth,DXGI_FORMAT format, int elementByte);
	/*
	* 한 resource에서 다른 resource로 data를 copy한다. (GPU->GPU)
	*/
	void										CopyUploadToBuffer(int src, int dst);
	/*
	* 입력된 value를 alignment한 값을 반환한다.
	*/
	int											CalculateAlignment(int value, int alignment);
	/*
	* index(handle)을 입력받아서 그에 해당하는 자원을 반환한다.
	*/
	ID3D12Resource*								GetResource(int index);
	/*
	* swapchain의 Present() 메서드를 실행한다.
	*/
	void										SwapChainPresent();
	/*
	* index(handle)에 해당하는 업로드 버퍼에 데이터를 copy한다.
	*/
	void										Upload(int index, const void* data, int byteSize,int offset);
	/*
	* index(handle)에 해당하는 업로드 버퍼에 texture2D 형식의 데이터를 copy한다.
	*/
	void										UploadTexture2D(int index, const void* data, int width, int height, int offsetX,int offsetY);

private:
	/*
	* 자원을 저장하고 있는 테이블이다.
	*/
	ResourceTable								mResourceTable;
	/*
	* 다음에 할당할 자원의 index(handle)이다.
	*/
	int											mNextResourceIdx = 0;
	/*
	* swapchain 자원이다.
	*/
	ComPtr<IDXGISwapChain>						mSwapChain;
};


static UINT constantBufferAlignment(UINT byteSize)
{
	return (byteSize + 255) & ~255;
}

