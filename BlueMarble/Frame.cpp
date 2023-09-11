#include "Frame.h"
#include "IfError.h"
#include "Constant.h"
#include "Engine.h"


Frame::Frame()
{
	IfError::Throw(Engine::mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(mCommandAllocator.GetAddressOf())),
		L"create command allocator error!");

	mObjConstantBufferIdx = Engine::mResourceManager->CreateUploadBuffer(constantBufferAlignment(sizeof(obj)*MAX_OBJECT_NUM));
	mEnvConstantBufferIdx = Engine::mResourceManager->CreateUploadBuffer(constantBufferAlignment(sizeof(env)));
}

ID3D12CommandAllocator* Frame::Get()
{
	return mCommandAllocator.Get();
}