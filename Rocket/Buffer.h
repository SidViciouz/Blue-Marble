#pragma once
#include "Util.h"

using namespace Microsoft::WRL;

class Buffer
{
public:
	Buffer(ID3D12Device* device);
private:
	ComPtr<ID3D12Resource> mBuffer;
	ComPtr<ID3D12Resource> mUploadBuffer;
};