#pragma once

#include "Util.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Constant.h"

class TextManager
{
public:
												TextManager();
	/*
	* text�� �����ϴ� �ؽ�ó�� srv index(handle)�� ��ȯ�Ѵ�.
	*/
	int											GetTextureSrvIdx() const;
	/*
	* text�� �������� �����ϴ� �ڿ��� srv index(handle)�� ��ȯ�Ѵ�.
	*/
	int											GetTextInfoSrvIdx() const;

protected:
	/*
	* freetype library�� FT_library ��ü�̴�.
	*/
	FT_Library									mLibrary;
	/*
	* freetype library�� FT_Face ��ü�̴�.
	*/
	FT_Face										mFace;
	/*
	* text�� bitmap�� �����ϴ� texture�� index(handle)�̴�.
	*/
	int											mTextureIdx;
	/*
	* text�� upload�Ҷ� ���Ǵ� upload buffer�� index(handle)�̴�.
	*/
	int											mUploadBufferIdx;
	/*
	* text�� �����ϴ� �ؽ�ó�� srv index(handle)�̴�.
	*/
	int											mTextureSrvIdx;
	/*
	* ascii�ڵ��� 0������ 127�������� �ؽ�Ʈ���� texture�ڿ��� ��ϵ� �������� �����ϰ� �ִ�.
	*/
	TextInfo									mTextInfo[128];
	/*
	* text�� ������ �����ϰ� �ִ� buffer�� ���� index(handle)�̴�.
	*/
	int											mTextInfoBufferIdx;
	/*
	* text�� ������ �����ϰ� �ִ� buffer�� ���ε��ϱ� ���� upload buffer�� index(handle)�̴�.
	*/
	int											mTextInfoUploadBufferIdx;
	/*
	* text�� ������ �����ϰ� �ִ� buffer�� ���� srv index(handle)�̴�.
	*/
	int											mTextInfoSrvIdx;
};
