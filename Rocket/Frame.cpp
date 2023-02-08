#include "Frame.h"
#include "IfError.h"

Frame::Frame(ID3D12Device* device)
{
	IfError::Throw(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(mCommandAllocator.GetAddressOf())),
		L"create command allocator error!");
}