#pragma once

#include "Util.h"
#include "IfError.h"

using namespace Microsoft::WRL;

class Pipeline
{
private:
	ComPtr<ID3D12Device> mDevice = nullptr;
	ComPtr<ID3D12Fence> mFence = nullptr;

	UINT64 mFenceValue = 0;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mDepthStencilBufferFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


public:
	void Initialize();
};
