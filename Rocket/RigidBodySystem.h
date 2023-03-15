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


	static vector<RigidBody*>					mRigidBodies;

protected:
	unique_ptr<TextureResource>					mRigidBodyPosTexture; //texture 2d array로 uav를 생성한다. (write용, read용이 필요하기 때문이다.)
	unique_ptr<TextureResource>					mRigidBodyQuatTexture; //quaternion
	unique_ptr<TextureResource>					mRigidBodyLMTexture; //linear momentum
	unique_ptr<TextureResource>					mRigidBodyAMTexture; //angular momentum
	unique_ptr<TextureResource>					mParticleCOMTexture;
	unique_ptr<TextureResource>					mParticlePosTexture;
	unique_ptr<TextureResource>					mParticleVelTexture;
	unique_ptr<TextureResource>					mGrid;	// texture 3d array로 생성한다.
	/*
	* particle개수 int 1개 + 현재 rigid body의 particle offset int 1개 -> stride는 2이다.
	*/
	unique_ptr<TextureResource>					mRigidInfos;
	/*
	* initial_inertia float3 *3개 current_inertia float3 * 3개 -> stride는 6이다.
	*/
	unique_ptr<TextureResource>					mRigidInertia;

	unique_ptr<TextureResource>					mDepthTexture;

	ComPtr<ID3D12DescriptorHeap>				mDsvHeap;
	ComPtr<ID3D12DescriptorHeap>				mSrvUavHeap;

	D3D12_VIEWPORT								mViewport;
	D3D12_RECT									mScissor;

	UINT										mDsvIncrementSize;
	UINT										mSrvUavIncrementSize;

	int											mCurrentTexture = 0;

	/*
	* 입력된 rigidBody에 대한 depth peeling을 통해 4개의 depth buffer에 깊이 값들을 만든다.
	*/
	void										DepthPass(RigidBody* rigidBody);
	/*
	* depth buffer들에서 particle의 개수와 위치를 파악해서 mParticleCOMTexture에 업로드한다.
	*/
	void										UploadParticleFromDepth(int index);
	/*
	* mRigidBodyPosTexture와 mRigidBodyQuatTexture, mParticleCOMTexture를 이용해서 particle들의 position을
	* 계산하여, mParticlePosTexture에 저장한다.
	*/
	void										CalculateParticlePosition(int objNum);
	/*
	* mRigidBodyLMTexture와 mRigidBodyAMTexture, mParticleCOMTexture를 이용해서 particle들의 velocity를
	* 계산하여, mParticleVelTexture에 저장한다.
	*/
	void										CalculateParticleVelocity(int objNum);
	/*
	* rigidbody의 inertia를 계산해서 mRigidInfos에 저장한다.
	*/
	void										CalculateRigidInertia(int objNum);
	/*
	* particle들을 grid에 올린다.
	*/
	void										PutParticleOnGrid(int objNum);

};


