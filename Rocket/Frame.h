#pragma once
#include "Util.h"
#include "Buffer.h"
#include "TextureResource.h"

using namespace Microsoft::WRL;
using namespace std;

class Frame
{
public:
												Frame();
	ID3D12CommandAllocator*						Get();
	UINT64										mFenceValue = 0;
	
	int											mObjConstantBufferIdx;
	int											mEnvConstantBufferIdx;

private:
	ComPtr<ID3D12CommandAllocator>				mCommandAllocator = nullptr;
};