#pragma once

#include "Util.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Constant.h"

class TextManager
{
public:
												TextManager();
	int											GetTextureSrvIdx() const;
	int											GetTextInfoSrvIdx() const;

protected:
	FT_Library									mLibrary;
	FT_Face										mFace;

	int											mTextureIdx;
	int											mUploadBufferIdx;
	int											mTextureSrvIdx;

	TextInfo									mTextInfo[128];

	int											mTextInfoBufferIdx;
	int											mTextInfoUploadBufferIdx;
	int											mTextInfoSrvIdx;
};
