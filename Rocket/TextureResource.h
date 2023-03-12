#pragma once
#include "Util.h"

//일단 2d텍스처를 다룬다.
class TextureResource
{
public:
												TextureResource() = default;
												TextureResource(const TextureResource& t) = delete;
												TextureResource(TextureResource&& t) = delete;
	TextureResource&							operator=(const TextureResource& t) = delete;
	TextureResource&							operator=(TextureResource&& t) = delete;

	void										Copy(void* pData,int width, int height, int elementByte,DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);
	void										Copy(void* pData, int width, int height, int depth, int elementByte, bool isArray = false,DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);
	void										CreateDepth(int width, int height, int depth, int elementByte);
	void										Create(int width, int height, int depth, int elementByte, bool isArray = false, DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);

	ComPtr<ID3D12Resource>						mUploadBuffer;
	ComPtr<ID3D12Resource>						mTexture;

};
