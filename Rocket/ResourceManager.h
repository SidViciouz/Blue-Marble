#pragma once

#include "Util.h"

#define MAX_RESOURCE_SIZE 1000

using ResourceTable = array<ComPtr<ID3D12Resource>, MAX_RESOURCE_SIZE>;

class ResourceManager
{
public:
	/*
	* swapchain�� �����Ѵ�.
	*/
	int											CreateSwapChain(int width, int height, DXGI_FORMAT format, HWND windowHandle);
	/*
	* upload buffer�� ������ ũ��� �����Ѵ�.
	*/
	int											CreateUploadBuffer(int byteSize);
	/*
	* default buffer�� ������ ũ��� �����Ѵ�.
	*/
	int											CreateDefaultBuffer(int byteSize);
	/*
	* 1���� �ؽ�ó�� �����Ѵ�.
	*/
	int											CreateTexture1D(int width, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	/*
	* 2���� �ؽ�ó�� �����Ѵ�.
	*/
	int											CreateTexture2D(int width,int height,DXGI_FORMAT format,D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	/*
	* 2���� �ؽ�ó�� �迭�� �����Ѵ�.
	*/
	int											CreateTexture2DArray(int width, int height, int arraySize, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	/*
	* 3���� �ؽ�ó�� �����Ѵ�.
	*/
	int											CreateTexture3D(int width, int height, int depth, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);
	/*
	* �� resource���� �ٸ� resource�� texture data�� copy�Ѵ�. (GPU->GPU)
	*/
	void										CopyUploadToTexture(int src,int dst,int width,int height,int depth,DXGI_FORMAT format, int elementByte);
	/*
	* �� resource���� �ٸ� resource�� data�� copy�Ѵ�. (GPU->GPU)
	*/
	void										CopyUploadToBuffer(int src, int dst);
	/*
	* �Էµ� value�� alignment�� ���� ��ȯ�Ѵ�.
	*/
	int											CalculateAlignment(int value, int alignment);
	/*
	* index(handle)�� �Է¹޾Ƽ� �׿� �ش��ϴ� �ڿ��� ��ȯ�Ѵ�.
	*/
	ID3D12Resource*								GetResource(int index);
	/*
	* swapchain�� Present() �޼��带 �����Ѵ�.
	*/
	void										SwapChainPresent();
	/*
	* index(handle)�� �ش��ϴ� ���ε� ���ۿ� �����͸� copy�Ѵ�.
	*/
	void										Upload(int index, const void* data, int byteSize,int offset);
	/*
	* index(handle)�� �ش��ϴ� ���ε� ���ۿ� texture2D ������ �����͸� copy�Ѵ�.
	*/
	void										UploadTexture2D(int index, const void* data, int width, int height, int offsetX,int offsetY);

private:
	/*
	* �ڿ��� �����ϰ� �ִ� ���̺��̴�.
	*/
	ResourceTable								mResourceTable;
	/*
	* ������ �Ҵ��� �ڿ��� index(handle)�̴�.
	*/
	int											mNextResourceIdx = 0;
	/*
	* swapchain �ڿ��̴�.
	*/
	ComPtr<IDXGISwapChain>						mSwapChain;
};


static UINT constantBufferAlignment(UINT byteSize)
{
	return (byteSize + 255) & ~255;
}

