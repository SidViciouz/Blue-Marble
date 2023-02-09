#include "Buffer.h"

Buffer::Buffer(ID3D12Device* device, int byteSize) // template������ declaration�� definition�� ������ �� ����.
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

	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//�� ���۴� upload buffer���� �����͸� copy�Ҷ��� destination���� ���δ�.
	//�̷��� ���۴� common state�� �ΰ�, copy ���� copy dest�� state�� transition������Ѵ�. (documentaion����)
	IfError::Throw(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(mBuffer.GetAddressOf())),
		L"create buffer error!");
}

void Buffer::Copy(const void* data, int byteSize)
{

}
