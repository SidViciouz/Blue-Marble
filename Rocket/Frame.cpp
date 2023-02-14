#include "Frame.h"
#include "IfError.h"
#include "Constant.h"

Frame::Frame(ID3D12Device* device)
{
	IfError::Throw(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(mCommandAllocator.GetAddressOf())),
		L"create command allocator error!");
	
	//object가 두개이기 때문에 sizeof(obj)*2를 기준으로 constant buffer를 생성함. (추후 수정해야함.)
	mObjConstantBuffer = make_unique<UploadBuffer>(device, BufferInterface::ConstantBufferByteSize(sizeof(obj))*3);
	mTransConstantBuffer = make_unique<UploadBuffer>(device, BufferInterface::ConstantBufferByteSize(sizeof(trans)));
}

ID3D12CommandAllocator* Frame::Get()
{
	return mCommandAllocator.Get();
}