#pragma once
#include "Util.h"
#include "IfError.h"

using namespace Microsoft::WRL;

class Buffer
{
public:
	Buffer(ID3D12Device* device, int byteSize);
	Buffer(const Buffer& buffer) = delete;
	Buffer(Buffer&& buffer) = delete;
	Buffer& operator=(const Buffer& buffer) = delete;
	Buffer& operator=(Buffer&& buffer) = delete;

	void Copy(const void* data, int byteSize,ID3D12GraphicsCommandList* commandList);

private:
	ComPtr<ID3D12Resource> mBuffer;
	ComPtr<ID3D12Resource> mUploadBuffer;
};