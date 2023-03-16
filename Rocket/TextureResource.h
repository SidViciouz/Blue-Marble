#pragma once
#include "Util.h"

//�ϴ� 2d�ؽ�ó�� �ٷ��.
class TextureResource
{
public:
												TextureResource() = default;
												TextureResource(const TextureResource& t) = delete;
												TextureResource(TextureResource&& t) = delete;
	TextureResource&							operator=(const TextureResource& t) = delete;
	TextureResource&							operator=(TextureResource&& t) = delete;

	/*
	* 2d�ؽ�ó�� ���ε� ���۸� �����Ѵ�.
	*/
	void										CopyCreate(void* pData,int width, int height, int elementByte,DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);
	/*
	* 3d�ؽ�ó �Ǵ� 2dArray�ؽ�ó�� ���ε� ���۸� �����Ѵ�.
	*/
	void										CopyCreate(void* pData, int width, int height, int depth, int elementByte, bool isArray = false,DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);
	/*
	* ���ε���ۿ� �����͸� ���ε��ϰ�, �ؽ�ó�� ī���Ѵ�.
	*/
	void										Copy(void* pData, int width, int height, int depth, int elementByte, DXGI_FORMAT format = DXGI_FORMAT_R32_FLOAT);
	/*
	* �ؽ�ó�� �����͸� Readback buffer�� ī���Ѵ�.
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
