#pragma once

#include "Util.h"
#include "IfError.h"
#include "Frame.h"

using namespace Microsoft::WRL;
using namespace std;

class Pipeline
{
private:
	void CreateCommandObjects();
	void CreateDescriptorHeaps();
	void CreateSwapChain();

	ComPtr<ID3D12Device> mDevice = nullptr;

	ComPtr<ID3D12Fence> mFence = nullptr;
	UINT64 mFenceValue = 0;

	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	bool mMsaaEnable = false;
	UINT mMsaaQuality = 0;

	vector<unique_ptr<Frame>> mFrames;
	int mNumberOfFrames = 3;
	int mCurrentFrame = 0;

	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12CommandList> mCommandList;

	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;

public:
	void Initialize();
};
