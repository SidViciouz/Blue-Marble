#include "Frame.h"
#include "IfError.h"
#include "Constant.h"
#include "Engine.h"

Frame::Frame(int numModels)
{
	IfError::Throw(Engine::mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(mCommandAllocator.GetAddressOf())),
		L"create command allocator error!");
	
	mObjConstantBuffer = make_unique<UploadBuffer>(Engine::mDevice.Get(), BufferInterface::ConstantBufferByteSize(sizeof(obj))* numModels);
	mTransConstantBuffer = make_unique<UploadBuffer>(Engine::mDevice.Get(), BufferInterface::ConstantBufferByteSize(sizeof(trans)));
}

ID3D12CommandAllocator* Frame::Get()
{
	return mCommandAllocator.Get();
}

void Frame::CopyObjConstantBuffer(int index, const void* data, int byteSize)
{
	mObjConstantBuffer->Copy(index, data, byteSize);
}

void Frame::CopyTransConstantBuffer(int index, const void* data, int byteSize)
{
	mTransConstantBuffer->Copy(index, data, byteSize);
}