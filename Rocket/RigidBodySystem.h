#pragma once

#include "Util.h"
#include "TextureResource.h"
#include "RigidBody.h"


class RigidBodySystem
{
public:
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
	unique_ptr<TextureResource>					mRigidBodyTexture; //texture 2d array�� uav�� �����Ѵ�. (write��, read���� �ʿ��ϱ� �����̴�.)
	unique_ptr<TextureResource>					mParticleTexture; //���� ����
	unique_ptr<TextureResource>					mGrid;	// texture 3d array�� �����Ѵ�.

	int											mCurrentTexture = 0;
};

