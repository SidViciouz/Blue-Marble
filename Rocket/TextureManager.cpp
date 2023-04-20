#include "TextureManager.h"
#include "Engine.h"

TextureManager::TextureManager()
{

}

void TextureManager::Load(const string& name,const wchar_t* path)
{
	ComPtr<ID3D12Resource> texture;
	ComPtr<ID3D12Resource> uploadBuffer;

	IfError::Throw(CreateDDSTextureFromFile12(
		Engine::mDevice.Get(),
		Engine::mCommandList.Get(),
		path,
		texture,
		uploadBuffer
	),
		L"create dds texture error!");
	
	mTexture[name] = move(texture);
	mUploadBuffer[name] = move(uploadBuffer);

	ID3D12Resource* resource = GetResource(name);

	mTextureIdx[name] =  Engine::mDescriptorManager->CreateSrv(resource, resource->GetDesc().Format, D3D12_SRV_DIMENSION_TEXTURE2D);
}

ID3D12Resource* TextureManager::GetResource(const string& name) const
{
	return mTexture.at(name).Get();
}


int TextureManager::GetTextureIndex(const string& name) const
{
	return mTextureIdx.at(name);
}