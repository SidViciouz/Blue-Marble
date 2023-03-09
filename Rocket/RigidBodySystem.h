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
	* mRigidBodies�� �����ϴ� rigid body�� mRigidBodyTexture�� ���ε��Ѵ�.
	*/
	void										Load();
	/*
	* load�� rigid body��� particle�� �����Ŀ� mParticleTexture�� ���ε��Ѵ�.
	*/
	void										GenerateParticle();

	void										DepthPass(RigidBody* rigidBody);

	static vector<RigidBody*>					mRigidBodies;

protected:
	unique_ptr<TextureResource>					mRigidBodyTexture; //texture 2d array�� uav�� �����Ѵ�. (write��, read���� �ʿ��ϱ� �����̴�.)
	unique_ptr<TextureResource>					mParticleTexture; //���� ����
	unique_ptr<TextureResource>					mGrid;	// texture 3d array�� �����Ѵ�.

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

