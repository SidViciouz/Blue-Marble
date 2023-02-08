#pragma once
#include "Util.h"

using namespace Microsoft::WRL;

class Frame
{
public:
	Frame(ID3D12Device* device);

private:
	ComPtr<ID3D12CommandAllocator> mCommandAllocator = nullptr;
	UINT64 mFenceValue = 0;
};
