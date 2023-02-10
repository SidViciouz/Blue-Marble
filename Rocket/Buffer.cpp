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
	resourceDesc.Width = byteSize; //������ ����Ʈ���� ����.
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//upload���� ������ �� resource state�� generic read������. (documentation����)
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

Buffer::Buffer(ID3D12Device* device, int byteSize) // template������ declaration�� definition�� ������ �� ����.
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
	resourceDesc.Width = byteSize; //������ ����Ʈ���� ����.
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	//�� ���۴� upload buffer���� �����͸� copy�Ҷ��� destination���� ���δ�.
	//�̷��� ���۴� common state�� �ΰ�, copy ���� copy dest�� state�� transition������Ѵ�. (documentaion����)
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
