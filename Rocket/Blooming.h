#pragma once

class Blooming
{
public:
												Blooming(int width,int height);
												Blooming(const Blooming& b) = delete;
												Blooming(Blooming&& b) = delete;
	Blooming&									operator=(const Blooming& b) = delete;
	Blooming&									operator=(Blooming&& b) = delete;

	void										Resize(int width,int height);
	void										DownScalePass();
	void										BrightPass();
	void										BlurPass();

protected:
	int											mDownScaledTextureIdx;
	int											mDownScaledTextureUavIdx;

	int											mBrightTextureIdx;
	int											mBrightTextureUavIdx;

	int											mHorizontalBlurredTextureIdx;
	int											mHorizontalBlurredTextureUavIdx;

	int											mBlurredTextureIdx;
	int											mBlurredTextureUavIdx;

	int											mConvolvedTextureIdx;
	int											mConvolvedTextureUavIdx;

	int											mRenderTargetSrvIdx[2];

	int											mWidth;
	int											mHeight;

};
