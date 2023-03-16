#include "TextureResource.h"
#include "d3dx12.h"
#include "IfError.h"
#include "Game.h"


int CalculateAlignment(int value, int alignment)
{
	//alignment가 2의 거듭제곱이여야한다.
	return (value + alignment - 1) & ~(alignment - 1);
}

TextureResource::~TextureResource()
{
	if(mUploadBuffer != nullptr)
		mUploadBuffer->Unmap(0, nullptr);
	if(mReadbackBuffer != nullptr)
		mReadbackBuffer->Unmap(0, nullptr);
}

//한번만 호출해야한다.
void TextureResource::CopyCreate(void* pData, int width, int height, int elementByte, DXGI_FORMAT format)
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

	UINT8* pBegin;

	IfError::Throw(mUploadBuffer->Map(0, &range, &pDataBegin),
		L"map to upload buffer error!");

	pBegin = reinterpret_cast<UINT8*>(pDataBegin);

	memcpy(pBegin, pData, sizeof(int) * 1000);

	//readback buffer 생성하는 부분
	hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
	rd = CD3DX12_RESOURCE_DESC::Buffer((UINT64)bufferSizeInBytes);
	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp, D3D12_HEAP_FLAG_NONE,
		&rd, D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mReadbackBuffer.GetAddressOf())
	),
		L"create readback buffer for texture resource error!"
	);

	//texture 생성하는 부분
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

	//cpu virtual memory address unmap하는 부분
	//mUploadBuffer->Unmap(0, nullptr);
}

//한번만 호출해야한다.
void TextureResource::CopyCreate(void* pData, int width, int height, int depth, int elementByte, bool isArray, DXGI_FORMAT format)
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

	UINT8* pBegin;

	IfError::Throw(mUploadBuffer->Map(0, &range, &pDataBegin),
		L"map to upload buffer error!");

	pBegin = reinterpret_cast<UINT8*>(pDataBegin);

	if(format == DXGI_FORMAT_R32G32B32A32_FLOAT)
		memcpy(pBegin, pData, sizeof(float) * 1000);
	else
		memcpy(pBegin, pData, sizeof(int) * 1000);

	//readback buffer 생성하는 부분

	hp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
	rd = CD3DX12_RESOURCE_DESC::Buffer((UINT64)bufferSizeInBytes);
	IfError::Throw(Pipeline::mDevice->CreateCommittedResource(
		&hp,D3D12_HEAP_FLAG_NONE,
		&rd,D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr, IID_PPV_ARGS(mReadbackBuffer.GetAddressOf())
		),
		L"create readback buffer for texture resource error!"
	);

	D3D12_RANGE readbackRange = {};
	readbackRange.Begin = 0;
	readbackRange.End = bufferSizeInBytes;

	IfError::Throw(mReadbackBuffer->Map(0, &readbackRange, reinterpret_cast<void**>(pReadbackDataBegin)),
		L"map to read back buffer error!");

	//texture 생성하는 부분
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

	//cpu virtual memory address unmap하는 부분
	//mUploadBuffer->Unmap(0, nullptr);
}

void TextureResource::Copy(void* pData, int width, int height, int depth, int elementByte, DXGI_FORMAT format)
{
	D3D12_RANGE range = {};
	range.Begin = 0;
	range.End = 0;

	UINT8* pBegin;

	pBegin = reinterpret_cast<UINT8*>(pDataBegin);

	/*
	* 1000*sizeof()만큼 데이터를 카피하는 것에서
	* 256바이트씩 align된 형태로 카피하는 것으로 수정해야함.
	*/
	if (format == DXGI_FORMAT_R32G32B32A32_FLOAT)
		memcpy(pBegin, pData, sizeof(float) * 1000);
	else
		memcpy(pBegin, pData, sizeof(int) * 1000);


	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
	footPrint.Offset = 0;
	footPrint.Footprint.Depth = depth;
	footPrint.Footprint.Format = format;
	footPrint.Footprint.Height = height;
	footPrint.Footprint.Width = width;
	footPrint.Footprint.RowPitch = CalculateAlignment(width * elementByte, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	D3D12_TEXTURE_COPY_LOCATION tl = CD3DX12_TEXTURE_COPY_LOCATION(mTexture.Get(), 0);
	D3D12_TEXTURE_COPY_LOCATION ubl = CD3DX12_TEXTURE_COPY_LOCATION(mUploadBuffer.Get(), footPrint);

	D3D12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(),D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_DEST);
	Game::mCommandList->ResourceBarrier(1, &b);

	Game::mCommandList->CopyTextureRegion(
		&tl,
		0, 0, 0,
		&ubl,
		nullptr
	);

	b = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Game::mCommandList->ResourceBarrier(1, &b);
}

void TextureResource::Readback(void* pData, int width, int height, int depth, int elementByte, DXGI_FORMAT format)
{
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footPrint = {};
	footPrint.Offset = 0;
	footPrint.Footprint.Depth = depth;
	footPrint.Footprint.Format = format;
	footPrint.Footprint.Height = height;
	footPrint.Footprint.Width = width;
	footPrint.Footprint.RowPitch = CalculateAlignment(width * elementByte, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

	D3D12_TEXTURE_COPY_LOCATION tl = CD3DX12_TEXTURE_COPY_LOCATION(mTexture.Get(), 0);
	D3D12_TEXTURE_COPY_LOCATION rbbl = CD3DX12_TEXTURE_COPY_LOCATION(mReadbackBuffer.Get(), footPrint);

	D3D12_RESOURCE_BARRIER b = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,D3D12_RESOURCE_STATE_COPY_SOURCE);
	Game::mCommandList->ResourceBarrier(1, &b);

	Game::mCommandList->CopyTextureRegion(
		&rbbl,
		0, 0, 0,
		&tl,
		nullptr
	);

	b = CD3DX12_RESOURCE_BARRIER::Transition(mTexture.Get(),D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	Game::mCommandList->ResourceBarrier(1, &b);
	
	//Game::mCommandList->CopyResource(mReadbackBuffer.Get(), mTexture.Get());
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

void TextureResource::Create(int width, int height, int depth, int elementByte, bool isArray, DXGI_FORMAT format)
{
	D3D12_RESOURCE_DESC rd;

	//texture 생성하는 부분
	//float으로 생성함.
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