#pragma once

#include "Util.h"
#include "Model.h"
#include "ParticleField.h"
#include "TextureResource.h"

class RigidBody
{
public:
												RigidBody(Model* model);
												~RigidBody();
												RigidBody(const RigidBody& r) = delete;
												RigidBody(RigidBody&& r) = delete;
	RigidBody&									operator=(const RigidBody& r) = delete;
	RigidBody&									operator=(RigidBody&& r) = delete;


	void										SetVelocity(const XMFLOAT3& velocity);
	void										SetVelocity(const float& x, const float& y, const float& z);
	const XMFLOAT3&								GetVelocity() const;

	void										CreateParticles();

	void										Draw();
	void										Collision();

	void										DrawParticles();

	unique_ptr<TextureResource>					mRigidBodyTexture; //texture 2d array로 uav를 생성한다. (write용, read용이 필요하기 때문이다.)
	unique_ptr<TextureResource>					mParticleTexture; //위와 동일
	unique_ptr<TextureResource>					mGrid;	// texture 3d array로 생성한다.

	static vector<RigidBody*>					mRigidBodies;

protected:
	Model*										mModel;
	vector<Particle>							mParticles;
	XMFLOAT3									mVelocity;
	int											mCurrentTexture = 0;
};