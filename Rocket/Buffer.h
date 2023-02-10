#pragma once
#include "Util.h"
#include "IfError.h"

using namespace Microsoft::WRL;

class BufferInterface
{
public:
	BufferInterface(const BufferInterface& buffer) = delete;
	BufferInterface(BufferInterface&& buffer) = delete;
	BufferInterface& operator=(const BufferInterface& buffer) = delete;
	BufferInterface& operator=(BufferInterface&& buffer) = delete;

	virtual void Copy(const void* data, int byteSize, ID3D12GraphicsCommandList* commandList) = 0;

protected:
	BufferInterface(ID3D12Device* device, int byteSize);

	ComPtr<ID3D12Resource> mUploadBuffer;
};

class UploadBuffer : public BufferInterface
{
public:
	UploadBuffer(ID3D12Device* device, int byteSize);
	UploadBuffer(const UploadBuffer& buffer) = delete;
	UploadBuffer(UploadBuffer&& buffer) = delete;
	UploadBuffer& operator=(const UploadBuffer& buffer) = delete;
	UploadBuffer& operator=(UploadBuffer&& buffer) = delete;
	~UploadBuffer();
	virtual void Copy(const void* data, int byteSize);

protected:
	BYTE* mMapped = nullptr;
};

class Buffer : public BufferInterface
{
public:
	Buffer(ID3D12Device* device, int byteSize);
	Buffer(const Buffer& buffer) = delete;
	Buffer(Buffer&& buffer) = delete;
	Buffer& operator=(const Buffer& buffer) = delete;
	Buffer& operator=(Buffer&& buffer) = delete;

	virtual void Copy(const void* data, int byteSize, ID3D12GraphicsCommandList* commandList) override;

protected:
	ComPtr<ID3D12Resource> mBuffer;
};