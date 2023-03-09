#include "TextureResource.h"
#include "d3dx12.h"
#include "IfError.h"
#include "Game.h"

int CalculateAlignment(int value, int alignment)
{
	//alignment가 2의 거듭제곱이여야한다.
	return (value + alignment - 1) & ~(alignment - 1);
}

//한번만 호출해야한다.
void TextureResource::Copy(void* pData, int width, int height, int elementByte)
{
	//upload buffer 생성하는 부분

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

	if (pData != nullptr)
		memcpy(pBegin, pData, sizeof(int) * 11);


	//texture 생성하는 부분
	hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	rd = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UINT,width,height);
	rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

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
	
	Game::mCommandList->CopyTextureRegion(
		&tl,
		0, 0, 0,
		&ubl,
		nullptr
	);

	D3D12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Game::mCommandList->ResourceBarrier(1, &b);

	//cpu virtual memory address unmap하는 부분
	mUploadBuffer->Unmap(0, nullptr);
}

//한번만 호출해야한다.
void TextureResource::Copy(void* pData, int width, int height, int depth, int elementByte)
{
	//upload buffer 생성하는 부분

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

	if(pData != nullptr)
		memcpy(pBegin, pData, sizeof(int) * 3000);


	//texture 생성하는 부분
	hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	rd = CD3DX12_RESOURCE_DESC::Tex3D(DXGI_FORMAT_R8G8B8A8_UINT, width, height, depth);
	rd.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

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

	Game::mCommandList->CopyTextureRegion(
		&tl,
		0, 0, 0,
		&ubl,
		nullptr
	);

	D3D12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(),D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Game::mCommandList->ResourceBarrier(1, &b);

	//cpu virtual memory address unmap하는 부분
	mUploadBuffer->Unmap(0, nullptr);
}

void TextureResource::CreateDepth(int width, int height, int depth, int elementByte)
{
	//texture 생성하는 부분
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