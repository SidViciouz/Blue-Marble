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
	* text를 저장하는 텍스처의 srv index(handle)을 반환한다.
	*/
	int											GetTextureSrvIdx() const;
	/*
	* text의 정보들을 저장하는 자원의 srv index(handle)을 반환한다.
	*/
	int											GetTextInfoSrvIdx() const;

protected:
	/*
	* freetype library의 FT_library 객체이다.
	*/
	FT_Library									mLibrary;
	/*
	* freetype library의 FT_Face 객체이다.
	*/
	FT_Face										mFace;
	/*
	* text를 bitmap을 저장하는 texture의 index(handle)이다.
	*/
	int											mTextureIdx;
	/*
	* text를 upload할때 사용되는 upload buffer의 index(handle)이다.
	*/
	int											mUploadBufferIdx;
	/*
	* text를 저장하는 텍스처의 srv index(handle)이다.
	*/
	int											mTextureSrvIdx;
	/*
	* ascii코드의 0번부터 127번까지의 텍스트들이 texture자원에 기록된 정보들을 저장하고 있다.
	*/
	TextInfo									mTextInfo[128];
	/*
	* text의 정보를 저장하고 있는 buffer에 대한 index(handle)이다.
	*/
	int											mTextInfoBufferIdx;
	/*
	* text의 정보를 저장하고 있는 buffer에 업로드하기 위한 upload buffer의 index(handle)이다.
	*/
	int											mTextInfoUploadBufferIdx;
	/*
	* text의 정보를 저장하고 있는 buffer에 대한 srv index(handle)이다.
	*/
	int											mTextInfoSrvIdx;
};
