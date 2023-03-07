#pragma once

#include "Util.h"

using namespace Microsoft::WRL;

class Texture
{
public:
	ComPtr<ID3D12Resource>						mResource = nullptr;
	ComPtr<ID3D12Resource>						mUpload = nullptr;
};