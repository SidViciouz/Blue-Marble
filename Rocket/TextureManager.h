#pragma once

#include "Util.h"
#include "DDSTextureLoader.h"

/*
* mesh�� ���� texture�� �����ϴ� ��ü�̴�.
* �̸� ������ �ٸ� �ڿ����� ��� resourceManager���� �����Ѵ�.
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
	* mesh�� ���� texture�� load�Ѵ�.
	*/
	void										Load(const string& name,const wchar_t* path);
	/*
	* name�� �ش��ϴ� �ؽ�ó�� ��ȯ�Ѵ�.
	*/
	ID3D12Resource*								GetResource(const string& name) const;
	/*
	* name�� �ش��ϴ� �ؽ�ó srv�� index(handle)�� ��ȯ�Ѵ�.
	*/
	int											GetTextureIndex(const string& name) const;

protected:
	/*
	* texture���� �����ϴ� �ڷᱸ���̴�.
	*/
	unordered_map<string,ComPtr<ID3D12Resource>>				mTexture;
	/*
	* ���ε� ���۸� �����ϴ� �ڷᱸ���̴�.
	*/
	unordered_map<string,ComPtr<ID3D12Resource>>				mUploadBuffer;
	/*
	* �ؽ�ó srv�� index(handle)���� �����ϴ� �ڷᱸ���̴�.
	*/
	unordered_map< string,int>					mTextureIdx;
};
