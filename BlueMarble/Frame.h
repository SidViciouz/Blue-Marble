#pragma once
#include "Util.h"

using namespace Microsoft::WRL;
using namespace std;

class Frame
{
public:
												Frame();
	/*
	* 프레임의 command allocator를 반환한다.
	*/
	ID3D12CommandAllocator*						Get();
	/*
	* 이 프레임의 fence value이다.
	*/
	UINT64										mFenceValue = 0;
	/*
	* 프레임이 저장하고 있는 object의 정보를 나타내는 constant buffer의 index(handle)이다.
	*/
	int											mObjConstantBufferIdx;
	/*
	* 프레임이 저장하고 있는 view, projection 등의 정보를 나타내는 constant buffer의 index(handle)이다.
	*/
	int											mEnvConstantBufferIdx;

private:
	/*
	* command allocator이다.
	*/
	ComPtr<ID3D12CommandAllocator>				mCommandAllocator = nullptr;
};