#pragma once

#include "Util.h"
#include "IfError.h"

using namespace Microsoft::WRL;

class Pipeline
{
private:
	ComPtr<ID3D12Device> mDevice = nullptr;

public:
	void Initialize();
};
