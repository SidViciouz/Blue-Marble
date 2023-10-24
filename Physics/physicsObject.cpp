#include "PhysicsObject.h"
#include "Collider.h"

using namespace Physics;
using namespace Maths;

PhysicsObject::PhysicsObject(PhysicsType type) :
	mPhysicsType(type)
{

}

void PhysicsObject::SetCollider(Collider* collider)
{
	mCollider = collider;
}

Collider* PhysicsObject::GetCollider() const
{
	return mCollider;
}

Transform& PhysicsObject::GetTransform()
{
	return mTransform;
}

void PhysicsObject::SetPhysicsType(PhysicsType type)
{
	mPhysicsType = type;
}

PhysicsType PhysicsObject::GetPhysicsType() const
{
	return mPhysicsType;
}


void PhysicsObject::SetInverseMass(float invMass)
{
	mInverseMass = invMass;
}

float PhysicsObject::GetInverseMass() const
{
	return mInverseMass;
}

Vector3 PhysicsObject::GetLinearVelocity() const {
	return mLinearVelocity;
}

Vector3 PhysicsObject::GetAngularVelocity() const {
	return mAngularVelocity;
}

Vector3 PhysicsObject::GetTorque() const
{
	return mTorque;
}

Vector3 PhysicsObject::GetForce() const
{
	return mForce;
}

void PhysicsObject::SetLinearVelocity(const Vector3& velocity)
{
	mLinearVelocity = velocity;
}

void PhysicsObject::SetAngularVelocity(const Vector3& velocity)
{
	mAngularVelocity = velocity;
}


Matrix3x3 PhysicsObject::GetInertiaTensor() const
{
	return mInverseInteriaTensor;
}

void PhysicsObject::ApplyAngularImpulse(const Vector3& force)
{
	mAngularVelocity += mInverseInteriaTensor * force;
}

void PhysicsObject::ApplyLinearImpulse(const Vector3& force)
{
	mLinearVelocity += force * mInverseMass;
}

void PhysicsObject::AddForce(const Vector3& addedForce)
{
	mForce += addedForce;
}

void PhysicsObject::AddTorque(const Vector3& addedTorque)
{
	mTorque += addedTorque;
}

void PhysicsObject::ClearForces()
{
	mForce = Vector3();
	mTorque = Vector3();
}

void PhysicsObject::InitializeCubeInertia()
{
	Vector3 dimensions = mTransform.GetScale();

	Vector3 dimsionSqr = dimensions * dimensions * 4;

	float im = 12.0f * mInverseMass;

	mInverseInertia.v.x = im / (dimsionSqr.v.y + dimsionSqr.v.z);
	mInverseInertia.v.y = im / (dimsionSqr.v.x + dimsionSqr.v.z);
	mInverseInertia.v.z = im / (dimsionSqr.v.x + dimsionSqr.v.y);
}


void PhysicsObject::UpdateInertiaTensor()
{
	Quaternion q = mTransform.GetOrientation();

	Matrix3x3 invOrientation = Matrix3x3(q.Conjugate());
	Matrix3x3 orientation = Matrix3x3(q);

	mInverseInteriaTensor = orientation * Matrix3x3::Scale(mInverseInertia) * invOrientation;
}