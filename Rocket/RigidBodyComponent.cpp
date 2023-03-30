#include "RigidBodyComponent.h"
#include "SceneNode.h"

RigidBodyComponent::RigidBodyComponent(shared_ptr<SceneNode> NodeAttachedTo,float mass)
	: mNodeAttachedTo{ NodeAttachedTo }, mMass{ mass }
{
	mPosition.v = mNodeAttachedTo->mAccumulatedPosition.Get();
	mRotation = mNodeAttachedTo->mAccumulatedQuaternion;
}

void RigidBodyComponent::Update(float deltaTime)
{
	// update linear properties
	mPosition.v = mNodeAttachedTo->mAccumulatedPosition.Get();

	Vector3 deltaVel = mForce * (deltaTime / mMass);

	mVelocity = mVelocity + deltaVel;

	mPosition = mPosition + (deltaVel * deltaTime);


	// update angular properties
	mRotation = mNodeAttachedTo->mAccumulatedQuaternion;

	Vector3 deltaAngularVel = (mTorque * deltaTime) * mInvInertiaTensor;

	mAngularVel = mAngularVel + deltaAngularVel;

	float deltaAngle = (deltaAngularVel * deltaTime).length() / 2.0f;

	Vector3 vec = deltaAngularVel.normalize() * sinf(deltaAngle);
	Quaternion deltaQuat( vec.v.x, vec.v.y, vec.v.z, cosf(deltaAngle));

	mRotation.Mul(deltaQuat);
}

void RigidBodyComponent::AddForce(Vector3 force, Vector3 relativePosition)
{
	mForce = mForce + force;
	mTorque = mTorque + relativePosition ^ force;
}