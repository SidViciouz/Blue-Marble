#include "Buffer.h"
#include "d3dx12.h"

BufferInterface::BufferInterface(ID3D12Device* device, int byteSize)
{
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.VisibleNodeMask = 0;
	heapProperties.CreationNodeMask = 0;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = byteSize; //버퍼의 바이트수를 지정.
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//upload힙을 생성할 때 resource state는 generic read여야함. (documentation참고)
	IfError::Throw(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(mUploadBuffer.GetAddressOf())),
		L"create upload buffer error!");
}

UploadBuffer::UploadBuffer(ID3D12Device* device, int byteSize)
	: BufferInterface(device,byteSize)
{
	mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMapped));
}

UploadBuffer::~UploadBuffer()
{
	if (mUploadBuffer != nullptr)
	{
		mUploadBuffer->Unmap(0, nullptr);
	}

	mMapped = nullptr;
}

void UploadBuffer::Copy(const void* data, int byteSize)
{
	memcpy(mMapped, data, byteSize);
}

Buffer::Buffer(ID3D12Device* device, int byteSize) // template에서는 declaration과 definition을 구분할 수 없다.
	: BufferInterface(device, byteSize)
{
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.VisibleNodeMask = 0;
	heapProperties.CreationNodeMask = 0;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = byteSize; //버퍼의 바이트수를 지정.
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//이 버퍼는 upload buffer에서 데이터를 copy할때의 destination으로 쓰인다.
	//이러한 버퍼는 common state로 두고, copy 전에 copy dest로 state를 transition해줘야한다. (documentaion참고)
	IfError::Throw(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(mBuffer.GetAddressOf())),
		L"create buffer error!");
}

void Buffer::Copy(const void* data, int byteSize, ID3D12GraphicsCommandList* commandList)
{
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = data;
	subresourceData.RowPitch = byteSize;
	subresourceData.SlicePitch = byteSize;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = mBuffer.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);

	UpdateSubresources<1>(commandList, mBuffer.Get(), mUploadBuffer.Get(), 0, 0, 1, &subresourceData);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;

	commandList->ResourceBarrier(1, &barrier);
}
