#pragma once

#include "Util.h"

class ShadowMap
{
public:
												ShadowMap(int width,int height);
	/*
	* shadowMap을 랜더링하기 위한 pipeline상태 setting한다.
	*/
	void										PipelineSetting();
	/*
	* shadowMap의 texture Srv index(handle)을 반환한다.
	*/
	int											GetTextureSrvIdx() const;

protected:
	/*
	* shadow map을 나타내는 texture의 index(handle)이다.
	*/
	int											mTextureIdx;
	/*
	* texture의 srv를 나타내는 index(handle)이다.
	*/
	int											mTextureSrvIdx;
	/*
	* texture의 dsv를 나타내는 index(handle)이다.
	*/
	int											mTextureDsvIdx;
	/*
	* 텍스처의 너비이다.
	*/
	int											mWidth;
	/*
	* 텍스처의 높이이다.
	*/
	int											mHeight;
	/*
	* 텍스처에 shadowMap을 그릴때 사용하는 viewport이다.
	*/
	D3D12_VIEWPORT								mViewport;
	/*
	* 텍스처에 shadowMap을 그릴때 사용하는 scissor renctangle이다.
	*/
	D3D12_RECT									mScissor;
};
