#pragma once

#include "Util.h"

class ShadowMap
{
public:
												ShadowMap(int width,int height);
	void										PipelineSetting();

protected:

	int											mTextureIdx;
	int											mTextureSrvIdx;
	int											mTextureDsvIdx;
	int											mWidth;
	int											mHeight;
	D3D12_VIEWPORT								mViewport;
	D3D12_RECT									mScissor;
};
