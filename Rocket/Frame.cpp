#include "Frame.h"
#include "IfError.h"
#include "Constant.h"

Frame::Frame(ID3D12Device* device)
{
	IfError::Throw(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(mCommandAllocator.GetAddressOf())),
		L"create command allocator error!");
	
	mObjConstantBuffer = make_unique<UploadBuffer>(device, BufferInterface::ConstantBufferByteSize(sizeof(obj)));
	mTransConstantBuffer = make_unique<UploadBuffer>(device, BufferInterface::ConstantBufferByteSize(sizeof(trans)));
}

ID3D12CommandAllocator* Frame::Get()
{
	return mCommandAllocator.Get();
}