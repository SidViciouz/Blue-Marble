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
	/*
	* 입력된 rigidBody에 대한 depth peeling을 통해 4개의 depth buffer에 깊이 값들을 만든다.
	*/
	void										DepthPass(RigidBody* rigidBody);
	/*
	* depth buffer들에서 particle의 개수와 위치를 파악해서 mParticleCOMTexture에 업로드한다.
	*/
	void										UploadParticleFromDepth(int index);
	/*
	* rigid body의 위치와 쿼터니언, 선운동량과 각운동량을 업로드한다.
	*/
	void										UploadRigidBody();
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
	/*
	* collision을 적용한다.
	*/
	void										ParticleCollision();
	/*
	* particle마다 작용하는 force를 이용해서 rigidBody의 next linear, angular momentum을 계산한다.
	*/
	void										NextRigidMomentum(float deltaTime);
	/*
	* rigid body의 linear, angular momentum을 이용해서 rigid body의 position과 quaternion을 계산한다.
	*/
	void										NextRigidPosQuat(int objNum,float deltaTime);
	/*
	* 계산된 rigidbody의 위치,쿼터니언,선운동량과 각운동량을 cpu의 rigidbody에 업데이트한다.
	*/
	void										UpdateRigidBody();

protected:
	unique_ptr<TextureResource>					mRigidBodyPosTexture; //(write용)
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
	/*
	* 각 particle에 작용하는 force를 저장하는 텍스처이다.
	*/
	unique_ptr<TextureResource>					mParticleForce;

	unique_ptr<TextureResource>					mRigidBodyPosTexture2; //(read용)
	unique_ptr<TextureResource>					mRigidBodyQuatTexture2; //quaternion
	unique_ptr<TextureResource>					mRigidBodyLMTexture2; //linear momentum
	unique_ptr<TextureResource>					mRigidBodyAMTexture2; //angular momentum


	unique_ptr<TextureResource>					mDepthTexture;

	ComPtr<ID3D12DescriptorHeap>				mDsvHeap;
	ComPtr<ID3D12DescriptorHeap>				mSrvUavHeap;

	D3D12_VIEWPORT								mViewport;
	D3D12_RECT									mScissor;

	UINT										mDsvIncrementSize;
	UINT										mSrvUavIncrementSize;

	int											mCurrentTexture = 0;

};


