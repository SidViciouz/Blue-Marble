#include "TextureResource.h"
#include "d3dx12.h"
#include "IfError.h"
#include "Game.h"

int CalculateAlignment(int value, int alignment)
{
	//alignment�� 2�� �ŵ������̿����Ѵ�.
	return (value + alignment - 1) & ~(alignment - 1);
}

//�ѹ��� ȣ���ؾ��Ѵ�.
void TextureResource::Copy(void* pData, int width, int height, int elementByte, DXGI_FORMAT format)
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

	memcpy(pBegin, pData, sizeof(int) * 11);


	//texture �����ϴ� �κ�
	hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	rd = CD3DX12_RESOURCE_DESC::Tex2D(format,width,height);
	rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mTexture.GetAddressOf())
	), L"create texture for texture resource error!");

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
	footPrint.Offset = 0;
	footPrint.Footprint.Depth = 1;
	footPrint.Footprint.Format = format;
	footPrint.Footprint.Height = height;
	footPrint.Footprint.Width = width;
	footPrint.Footprint.RowPitch = CalculateAlignment(width * elementByte, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	D3D12_TEXTURE_COPY_LOCATION tl = CD3DX12_TEXTURE_COPY_LOCATION(mTexture.Get(), 0);
	D3D12_TEXTURE_COPY_LOCATION ubl = CD3DX12_TEXTURE_COPY_LOCATION(mUploadBuffer.Get(),footPrint );
	
	Game::mCommandList->CopyTextureRegion(
		&tl,
		0, 0, 0,
		&ubl,
		nullptr
	);

	D3D12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Game::mCommandList->ResourceBarrier(1, &b);

	//cpu virtual memory address unmap�ϴ� �κ�
	mUploadBuffer->Unmap(0, nullptr);
}

//�ѹ��� ȣ���ؾ��Ѵ�.
void TextureResource::Copy(void* pData, int width, int height, int depth, int elementByte, bool isArray, DXGI_FORMAT format)
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

	if(format == DXGI_FORMAT_R32G32B32A32_FLOAT)
		memcpy(pBegin, pData, sizeof(float) * 1000);
	else
		memcpy(pBegin, pData, sizeof(int) * 1000);

	//texture �����ϴ� �κ�
	hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if(isArray == false)
		rd = CD3DX12_RESOURCE_DESC::Tex3D(format, width, height, depth);
	else
		rd = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, depth);

	rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mTexture.GetAddressOf())
	), L"create texture for texture resource error!");

	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
	footPrint.Offset = 0;
	footPrint.Footprint.Depth = depth;
	footPrint.Footprint.Format = format;
	footPrint.Footprint.Height = height;
	footPrint.Footprint.Width = width;
	footPrint.Footprint.RowPitch = CalculateAlignment(width * elementByte, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	D3D12_TEXTURE_COPY_LOCATION tl = CD3DX12_TEXTURE_COPY_LOCATION(mTexture.Get(), 0);
	D3D12_TEXTURE_COPY_LOCATION ubl = CD3DX12_TEXTURE_COPY_LOCATION(mUploadBuffer.Get(), footPrint);

	Game::mCommandList->CopyTextureRegion(
		&tl,
		0, 0, 0,
		&ubl,
		nullptr
	);

	D3D12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(),D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Game::mCommandList->ResourceBarrier(1, &b);

	//cpu virtual memory address unmap�ϴ� �κ�
	mUploadBuffer->Unmap(0, nullptr);
}

void TextureResource::CreateDepth(int width, int height, int depth, int elementByte)
{
	//texture �����ϴ� �κ�
	D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_RESOURCE_DESC rd = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R24G8_TYPELESS, width, height, depth);
	rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue, IID_PPV_ARGS(mTexture.GetAddressOf())
	), L"create texture for depth error!");
}

void TextureResource::Create(int width, int height, int depth, int elementByte, bool isArray, DXGI_FORMAT format)
{
	D3D12_RESOURCE_DESC rd;

	//texture �����ϴ� �κ�
	//float���� ������.
	D3D12_HEAP_PROPERTIES hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	if (isArray == false)
		rd = CD3DX12_RESOURCE_DESC::Tex3D(format, width, height, depth);
	else
		rd = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, depth);

	rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr, IID_PPV_ARGS(mTexture.GetAddressOf())
	), L"create texture for texture resource error!");
}