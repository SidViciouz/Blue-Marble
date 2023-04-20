#pragma once
#include "Util.h"

using namespace Microsoft::WRL;
using namespace std;

class Frame
{
public:
												Frame();
	/*
	* �������� command allocator�� ��ȯ�Ѵ�.
	*/
	ID3D12CommandAllocator*						Get();
	/*
	* �� �������� fence value�̴�.
	*/
	UINT64										mFenceValue = 0;
	/*
	* �������� �����ϰ� �ִ� object�� ������ ��Ÿ���� constant buffer�� index(handle)�̴�.
	*/
	int											mObjConstantBufferIdx;
	/*
	* �������� �����ϰ� �ִ� view, projection ���� ������ ��Ÿ���� constant buffer�� index(handle)�̴�.
	*/
	int											mEnvConstantBufferIdx;

private:
	/*
	* command allocator�̴�.
	*/
	ComPtr<ID3D12CommandAllocator>				mCommandAllocator = nullptr;
};