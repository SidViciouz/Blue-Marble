#pragma once

#include "Util.h"
#include "IfError.h"
#include "Frame.h"
#include "Model.h"
#include <map>

using namespace Microsoft::WRL;
using namespace std;

class Pipeline
{
public:
	Pipeline(const int& width,const int& height);

private:
	void CreateCommandObjects();
	void CreateDescriptorHeaps();
	void CreateSwapChain(HWND windowHandle);
	void CreateShaderAndRootSignature();

	const int& mWidth;
	const int& mHeight;

	ComPtr<ID3D12Device> mDevice = nullptr;

	ComPtr<IDXGIFactory4> mFactory = nullptr;

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
	ComPtr<ID3D12GraphicsCommandList> mCommandList;

	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;

	ComPtr<IDXGISwapChain> mSwapChain;

	unordered_map<string, ComPtr<ID3DBlob>> mShaders;

	unordered_map<string,ComPtr<ID3D12RootSignature>> mRootSignatures;

public:
	void Initialize(HWND windowHandle);
};
