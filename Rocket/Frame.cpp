#include "Frame.h"
#include "IfError.h"
#include "Constant.h"
#include "Pipeline.h"

Frame::Frame(int numModels)
{
	IfError::Throw(Pipeline::mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(mCommandAllocator.GetAddressOf())),
		L"create command allocator error!");
	
	mObjConstantBuffer = make_unique<UploadBuffer>(Pipeline::mDevice.Get(), BufferInterface::ConstantBufferByteSize(sizeof(obj))* numModels);
	mTransConstantBuffer = make_unique<UploadBuffer>(Pipeline::mDevice.Get(), BufferInterface::ConstantBufferByteSize(sizeof(trans)));
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