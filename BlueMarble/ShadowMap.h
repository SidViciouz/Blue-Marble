#pragma once

#include "Util.h"

class ShadowMap
{
public:
												ShadowMap(int width,int height);
	/*
	* shadowMap�� �������ϱ� ���� pipeline���� setting�Ѵ�.
	*/
	void										PipelineSetting();
	/*
	* shadowMap�� texture Srv index(handle)�� ��ȯ�Ѵ�.
	*/
	int											GetTextureSrvIdx() const;

protected:
	/*
	* shadow map�� ��Ÿ���� texture�� index(handle)�̴�.
	*/
	int											mTextureIdx;
	/*
	* texture�� srv�� ��Ÿ���� index(handle)�̴�.
	*/
	int											mTextureSrvIdx;
	/*
	* texture�� dsv�� ��Ÿ���� index(handle)�̴�.
	*/
	int											mTextureDsvIdx;
	/*
	* �ؽ�ó�� �ʺ��̴�.
	*/
	int											mWidth;
	/*
	* �ؽ�ó�� �����̴�.
	*/
	int											mHeight;
	/*
	* �ؽ�ó�� shadowMap�� �׸��� ����ϴ� viewport�̴�.
	*/
	D3D12_VIEWPORT								mViewport;
	/*
	* �ؽ�ó�� shadowMap�� �׸��� ����ϴ� scissor renctangle�̴�.
	*/
	D3D12_RECT									mScissor;
};
