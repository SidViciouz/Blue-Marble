#pragma once

#include "Util.h"
#include "DDSTextureLoader.h"

/*
* mesh에 입힐 texture를 관리하는 객체이다.
* 이를 제외한 다른 자원들은 모두 resourceManager에서 관리한다.
*/
class TextureManager
{
public:
												TextureManager();
												TextureManager(const TextureManager& tm) = delete;
												TextureManager(TextureManager&& tm) = delete;
	TextureManager&								operator=(const TextureManager& tm) = delete;
	TextureManager&								operator=(TextureManager&& tm) = delete;

	/*
	* mesh에 입힐 texture를 load한다.
	*/
	void										Load(const string& name,const wchar_t* path);
	/*
	* name에 해당하는 텍스처를 반환한다.
	*/
	ID3D12Resource*								GetResource(const string& name) const;
	/*
	* name에 해당하는 텍스처 srv의 index(handle)을 반환한다.
	*/
	int											GetTextureIndex(const string& name) const;

protected:
	/*
	* texture들을 저장하는 자료구조이다.
	*/
	unordered_map<string,ComPtr<ID3D12Resource>>				mTexture;
	/*
	* 업로드 버퍼를 저장하는 자료구조이다.
	*/
	unordered_map<string,ComPtr<ID3D12Resource>>				mUploadBuffer;
	/*
	* 텍스처 srv의 index(handle)들을 저장하는 자료구조이다.
	*/
	unordered_map< string,int>					mTextureIdx;
};
