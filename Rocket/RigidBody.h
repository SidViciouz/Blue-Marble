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

	Model*										mModel;

protected:
	vector<Particle>							mParticles;
	XMFLOAT3									mVelocity;
};