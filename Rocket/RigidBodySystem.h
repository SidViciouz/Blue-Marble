#pragma once

#include "Util.h"
#include "TextureResource.h"
#include "RigidBody.h"


class RigidBodySystem
{
public:
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
	unique_ptr<TextureResource>					mRigidBodyTexture; //texture 2d array로 uav를 생성한다. (write용, read용이 필요하기 때문이다.)
	unique_ptr<TextureResource>					mParticleTexture; //위와 동일
	unique_ptr<TextureResource>					mGrid;	// texture 3d array로 생성한다.

	int											mCurrentTexture = 0;
};

