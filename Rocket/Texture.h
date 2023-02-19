#pragma once

#include "Util.h"

using namespace Microsoft::WRL;

class Texture
{
public:
	ComPtr<ID3D12Resource> mTexture = nullptr;
	ComPtr<ID3D12Resource> mUpload = nullptr;
};
