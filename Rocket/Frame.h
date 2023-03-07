#pragma once
#include "Util.h"
#include "Buffer.h"
#include "TextureResource.h"

using namespace Microsoft::WRL;
using namespace std;

class Frame
{
public:
	Frame(int numModels);
	ID3D12CommandAllocator* Get();
	UINT64 mFenceValue = 0;
	unique_ptr<UploadBuffer> mObjConstantBuffer = nullptr;
	unique_ptr<UploadBuffer> mTransConstantBuffer = nullptr;
	void CopyObjConstantBuffer(int index, const void* data, int byteSize);
	void CopyTransConstantBuffer(int index, const void* data, int byteSize);

private:
	ComPtr<ID3D12CommandAllocator> mCommandAllocator = nullptr;
};
