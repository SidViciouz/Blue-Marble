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

	unique_ptr<TextureResource>					mRigidBodyTexture; //texture 2d array�� uav�� �����Ѵ�. (write��, read���� �ʿ��ϱ� �����̴�.)
	unique_ptr<TextureResource>					mParticleTexture; //���� ����
	unique_ptr<TextureResource>					mGrid;	// texture 3d array�� �����Ѵ�.

	static vector<RigidBody*>					mRigidBodies;

protected:
	Model*										mModel;
	vector<Particle>							mParticles;
	XMFLOAT3									mVelocity;
	int											mCurrentTexture = 0;
};