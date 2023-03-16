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

	/*
	* 2d텍스처와 업로드 버퍼를 생성한다.
	*/
	void										CopyCreate(void* pData,int width, int height, int elementByte,DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);
	/*
	* 3d텍스처 또는 2dArray텍스처와 업로드 버퍼를 생성한다.
	*/
	void										CopyCreate(void* pData, int width, int height, int depth, int elementByte, bool isArray = false,DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);
	/*
	* 업로드버퍼에 데이터를 업로드하고, 텍스처로 카피한다.
	*/
	void										Copy(void* pData, int width, int height, int depth, int elementByte, DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);
	/*
	* 텍스처의 데이터를 Readback buffer로 카피한다.
	*/
	void										Readback();
	void										CreateDepth(int width, int height, int depth, int elementByte);
	void										Create(int width, int height, int depth, int elementByte, bool isArray = false, DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);

	ComPtr<ID3D12Resource>						mUploadBuffer;
	ComPtr<ID3D12Resource>						mTexture;
	ComPtr<ID3D12Resource>						mReadbackBuffer;
	
protected:
	void*										pDataBegin;
};
