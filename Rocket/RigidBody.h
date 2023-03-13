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

	void										SetLinearMomentum(const XMFLOAT3& linearMomentum);
	void										SetLinearMomentum(const float& x, const float& y, const float& z);
	const XMFLOAT3&								GetLinearMomentum() const;

	void										SetAngularMomentum(const XMFLOAT3& AngularMomentum);
	void										SetAngularMomentum(const float& x, const float& y, const float& z);
	const XMFLOAT3&								GetAngularMomentum() const;

	void										CreateParticles();

	void										Draw();
	void										Collision();

	void										DrawParticles();

	Model*										mModel;

protected:
	vector<Particle>							mParticles;
	XMFLOAT3									mLinearMomentum;
	XMFLOAT3									mAngularMomentum;
};