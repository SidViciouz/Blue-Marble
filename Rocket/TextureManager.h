#pragma once

#include "Util.h"
#include "DDSTextureLoader.h"

class TextureManager
{
public:
												TextureManager();
												TextureManager(const TextureManager& tm) = delete;
												TextureManager(TextureManager&& tm) = delete;
	TextureManager&								operator=(const TextureManager& tm) = delete;
	TextureManager&								operator=(TextureManager&& tm) = delete;

	void										Load(const string& name,const wchar_t* path);
	ID3D12Resource*								GetResource(const string& name) const;
	int											GetTextureIndex(const string& name) const;

protected:
	unordered_map<string,ComPtr<ID3D12Resource>>				mTexture;
	unordered_map<string,ComPtr<ID3D12Resource>>				mUploadBuffer;
	unordered_map< string,int>					mTextureIdx;
};
