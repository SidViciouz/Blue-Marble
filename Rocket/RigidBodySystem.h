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
	/*
	* �Էµ� rigidBody�� ���� depth peeling�� ���� 4���� depth buffer�� ���� ������ �����.
	*/
	void										DepthPass(RigidBody* rigidBody);
	/*
	* depth buffer�鿡�� particle�� ������ ��ġ�� �ľ��ؼ� mParticleCOMTexture�� ���ε��Ѵ�.
	*/
	void										UploadParticleFromDepth(int index);
	/*
	* rigid body�� ��ġ�� ���ʹϾ�, ������� ������� ���ε��Ѵ�.
	*/
	void										UploadRigidBody();
	/*
	* mRigidBodyPosTexture�� mRigidBodyQuatTexture, mParticleCOMTexture�� �̿��ؼ� particle���� position��
	* ����Ͽ�, mParticlePosTexture�� �����Ѵ�.
	*/
	void										CalculateParticlePosition(int objNum);
	/*
	* mRigidBodyLMTexture�� mRigidBodyAMTexture, mParticleCOMTexture�� �̿��ؼ� particle���� velocity��
	* ����Ͽ�, mParticleVelTexture�� �����Ѵ�.
	*/
	void										CalculateParticleVelocity(int objNum);
	/*
	* rigidbody�� inertia�� ����ؼ� mRigidInfos�� �����Ѵ�.
	*/
	void										CalculateRigidInertia(int objNum);
	/*
	* particle���� grid�� �ø���.
	*/
	void										PutParticleOnGrid(int objNum);
	/*
	* collision�� �����Ѵ�.
	*/
	void										ParticleCollision();
	/*
	* particle���� �ۿ��ϴ� force�� �̿��ؼ� rigidBody�� next linear, angular momentum�� ����Ѵ�.
	*/
	void										NextRigidMomentum(float deltaTime);
	/*
	* rigid body�� linear, angular momentum�� �̿��ؼ� rigid body�� position�� quaternion�� ����Ѵ�.
	*/
	void										NextRigidPosQuat(int objNum,float deltaTime);
	/*
	* ���� rigidbody�� ��ġ,���ʹϾ�,������� ������� cpu�� rigidbody�� ������Ʈ�Ѵ�.
	*/
	void										UpdateRigidBody();

protected:
	unique_ptr<TextureResource>					mRigidBodyPosTexture; //(write��)
	unique_ptr<TextureResource>					mRigidBodyQuatTexture; //quaternion
	unique_ptr<TextureResource>					mRigidBodyLMTexture; //linear momentum
	unique_ptr<TextureResource>					mRigidBodyAMTexture; //angular momentum
	unique_ptr<TextureResource>					mParticleCOMTexture;
	unique_ptr<TextureResource>					mParticlePosTexture;
	unique_ptr<TextureResource>					mParticleVelTexture;
	unique_ptr<TextureResource>					mGrid;	// texture 3d array�� �����Ѵ�.
	/*
	* particle���� int 1�� + ���� rigid body�� particle offset int 1�� -> stride�� 2�̴�.
	*/
	unique_ptr<TextureResource>					mRigidInfos;
	/*
	* initial_inertia float3 *3�� current_inertia float3 * 3�� -> stride�� 6�̴�.
	*/
	unique_ptr<TextureResource>					mRigidInertia;
	/*
	* �� particle�� �ۿ��ϴ� force�� �����ϴ� �ؽ�ó�̴�.
	*/
	unique_ptr<TextureResource>					mParticleForce;

	unique_ptr<TextureResource>					mRigidBodyPosTexture2; //(read��)
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


