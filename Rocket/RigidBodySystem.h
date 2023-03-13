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


	static vector<RigidBody*>					mRigidBodies;

protected:
	unique_ptr<TextureResource>					mRigidBodyPosTexture; //texture 2d array�� uav�� �����Ѵ�. (write��, read���� �ʿ��ϱ� �����̴�.)
	unique_ptr<TextureResource>					mRigidBodyQuatTexture; //quaternion
	unique_ptr<TextureResource>					mRigidBodyLMTexture; //linear momentum
	unique_ptr<TextureResource>					mRigidBodyAMTexture; //angular momentum
	unique_ptr<TextureResource>					mParticleCOMTexture;
	unique_ptr<TextureResource>					mParticlePosTexture;
	unique_ptr<TextureResource>					mParticleVelTexture;
	unique_ptr<TextureResource>					mGrid;	// texture 3d array�� �����Ѵ�.
	unique_ptr<TextureResource>					mRigidInfos;

	unique_ptr<TextureResource>					mDepthTexture;

	ComPtr<ID3D12DescriptorHeap>				mDsvHeap;
	ComPtr<ID3D12DescriptorHeap>				mSrvUavHeap;

	D3D12_VIEWPORT								mViewport;
	D3D12_RECT									mScissor;

	UINT										mDsvIncrementSize;
	UINT										mSrvUavIncrementSize;

	int											mCurrentTexture = 0;

	/*
	* �Էµ� rigidBody�� ���� depth peeling�� ���� 4���� depth buffer�� ���� ������ �����.
	*/
	void										DepthPass(RigidBody* rigidBody);
	/*
	* depth buffer�鿡�� particle�� ������ ��ġ�� �ľ��ؼ� mParticleTexture�� ���ε��Ѵ�.
	*/
	void										UploadParticleFromDepth(int index);
};

