#include "TextureResource.h"
#include "Pipeline.h"
#include "d3dx12.h"
#include "IfError.h"

int CalculateAlignment(int value, int alignment)
{
	//alignment�� 2�� ��������Ѵ�.
	return (value + alignment - 1) & ~(alignment - 1);
}

void TextureResource::Copy(void* pData, int width, int height, int elementByte)
{
	//upload buffer �����ϴ� �κ�

	int bufferSizeInBytes = (height - 1) * CalculateAlignment(width*elementByte, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) + width*elementByte;
	D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer((UINT64)bufferSizeInBytes);

	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(mUploadBuffer.GetAddressOf())
	),L"create upload buffer for texture resource error!");

	D3D12_RANGE range = {};
	range.Begin = 0;
	range.End = 0;

	void* pDataBegin;
	UINT8* pBegin;

	mUploadBuffer->Map(0, &range, &pDataBegin);

	pBegin = reinterpret_cast<UINT8*>(pDataBegin);
	
	const int d[11] = { 1,2,3,4,5,6,7,8,9,10,11 };

	memcpy(pBegin, d, sizeof(int) * 11);


	//texture �����ϴ� �κ�
	hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	rd = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UINT,width,height);


	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mTexture.GetAddressOf())
	), L"create texture for texture resource error!");

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
	footPrint.Offset = 0;
	footPrint.Footprint.Depth = 1;
	footPrint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	footPrint.Footprint.Height = height;
	footPrint.Footprint.Width = width;
	footPrint.Footprint.RowPitch = CalculateAlignment(width * elementByte, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	D3D12_TEXTURE_COPY_LOCATION tl = CD3DX12_TEXTURE_COPY_LOCATION(mTexture.Get(), 0);
	D3D12_TEXTURE_COPY_LOCATION ubl = CD3DX12_TEXTURE_COPY_LOCATION(mUploadBuffer.Get(),footPrint );
	
	Pipeline::mCommandList->CopyTextureRegion(
		&tl,
		0, 0, 0,
		&ubl,
		nullptr
	);

	//cpu virtual memory address unmap�ϴ� �κ�
	mUploadBuffer->Unmap(0, nullptr);
}

void TextureResource::Copy(void* pData, int width, int height, int depth, int elementByte)
{
	//upload buffer �����ϴ� �κ�

	int bufferSizeInBytes =  (width*height - 1) * CalculateAlignment(width * elementByte, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT) + width * elementByte;
	D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Buffer((UINT64)bufferSizeInBytes);

	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(mUploadBuffer.GetAddressOf())
	), L"create upload buffer for texture resource error!");

	D3D12_RANGE range = {};
	range.Begin = 0;
	range.End = 0;

	void* pDataBegin;
	UINT8* pBegin;

	mUploadBuffer->Map(0, &range, &pDataBegin);

	pBegin = reinterpret_cast<UINT8*>(pDataBegin);

	const int d[11] = { 1,2,3,4,5,6,7,8,9,10,11 };

	memcpy(pBegin, d, sizeof(int) * 11);


	//texture �����ϴ� �κ�
	hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	rd = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UINT, width, height, depth);


	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mTexture.GetAddressOf())
	), L"create texture for texture resource error!");

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
	footPrint.Offset = 0;
	footPrint.Footprint.Depth = depth;
	footPrint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UINT;
	footPrint.Footprint.Height = height;
	footPrint.Footprint.Width = width;
	footPrint.Footprint.RowPitch = CalculateAlignment(width * elementByte, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	D3D12_TEXTURE_COPY_LOCATION tl = CD3DX12_TEXTURE_COPY_LOCATION(mTexture.Get(), 0);
	D3D12_TEXTURE_COPY_LOCATION ubl = CD3DX12_TEXTURE_COPY_LOCATION(mUploadBuffer.Get(), footPrint);

	Pipeline::mCommandList->CopyTextureRegion(
		&tl,
		0, 0, 0,
		&ubl,
		nullptr
	);

	//cpu virtual memory address unmap�ϴ� �κ�
	mUploadBuffer->Unmap(0, nullptr);
}