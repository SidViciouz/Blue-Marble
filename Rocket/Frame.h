#pragma once
#include "Util.h"
#include "Buffer.h"

using namespace Microsoft::WRL;
using namespace std;

class Frame
{
public:
	Frame(ID3D12Device* device);
	ID3D12CommandAllocator* Get();

private:
	ComPtr<ID3D12CommandAllocator> mCommandAllocator = nullptr;
	UINT64 mFenceValue = 0;
	unique_ptr<UploadBuffer> mObjConstantBuffer = nullptr;
	unique_ptr<UploadBuffer> mTransConstantBuffer = nullptr;
};
