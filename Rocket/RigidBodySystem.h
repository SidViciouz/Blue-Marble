#pragma once

#include "Util.h"
#include "TextureResource.h"
#include "RigidBody.h"
#include "Buffer.h"


class RigidBodySystem
{
public:
												RigidBodySystem();
	/*
	* mRigidBodies에 존재하는 rigid body를 mRigidBodyTexture에 업로드한다.
	*/
	void										Load();
	/*
	* load된 rigid body들로 particle을 생성후에 mParticleTexture에 업로드한다.
	*/
	void										GenerateParticle();

	void										DepthPass(RigidBody* rigidBody);

	static vector<RigidBody*>					mRigidBodies;

protected:
	unique_ptr<TextureResource>					mRigidBodyTexture; //texture 2d array로 uav를 생성한다. (write용, read용이 필요하기 때문이다.)
	unique_ptr<TextureResource>					mParticleTexture; //위와 동일
	unique_ptr<TextureResource>					mGrid;	// texture 3d array로 생성한다.

	unique_ptr<TextureResource>					mDepthTexture;
	unique_ptr<TextureResource>					mPrevDepthTexture;

	ComPtr<ID3D12DescriptorHeap>				mUavHeap;
	ComPtr<ID3D12DescriptorHeap>				mInvisibleSrvHeap;
	ComPtr<ID3D12DescriptorHeap>				mDsvHeap;
	ComPtr<ID3D12DescriptorHeap>				mSrvHeap;

	D3D12_VIEWPORT								mViewport;
	D3D12_RECT									mScissor;

	int											mCurrentTexture = 0;
};

