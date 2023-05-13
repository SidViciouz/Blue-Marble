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

void PhysicsObject::SetWorldID(int newID)
{
	mWorldID = newID;
}

int PhysicsObject::GetWorldID() const
{
	return mWorldID;
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

void PhysicsObject::SetLinearVelocity(const Vector3& v)
{
	mLinearVelocity = v;
}

void PhysicsObject::SetAngularVelocity(const Vector3& v)
{
	mAngularVelocity = v;
}


Matrix3x3 PhysicsObject::GetInertiaTensor() const
{
	return mInverseInteriaTensor;
}

void PhysicsObject::SetElasticity(float e)
{ 
	mElasticity = e; 
}
float PhysicsObject::GetElasticity() const
{
	return mElasticity;
}

void PhysicsObject::ApplyAngularImpulse(const Vector3& force)
{
	if (force.Length() > 0) {
		bool a = true;
	}
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

void PhysicsObject::AddForceAtPosition(const Vector3& addedForce, const Vector3& position)
{
	Vector3 localPos = position - mTransform.GetPosition();

	mForce += addedForce;
	mTorque += Vector3::Cross(localPos, addedForce); // Why don't need to suit the Conversation of Energy?
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

void PhysicsObject::InitCubeInertia()
{
	Vector3 dimensions = mTransform.GetScale();

	Vector3 fullWidth = dimensions * 2;

	Vector3 dimsSqr = fullWidth * fullWidth;

	mInverseInertia.v.x = (12.0f * mInverseMass) / (dimsSqr.v.y + dimsSqr.v.z);
	mInverseInertia.v.y = (12.0f * mInverseMass) / (dimsSqr.v.x + dimsSqr.v.z);
	mInverseInertia.v.z = (12.0f * mInverseMass) / (dimsSqr.v.x + dimsSqr.v.y);
}

void PhysicsObject::InitSphereInertia()
{
	float radius = mTransform.GetScale().GetMaxElement();
	float i = 2.5f * mInverseMass / (radius * radius);

	mInverseInertia = Vector3(i, i, i);
}

void PhysicsObject::UpdateInertiaTensor()
{
	Quaternion q = mTransform.GetOrientation();

	Matrix3x3 invOrientation = Matrix3x3(q.Conjugate());
	Matrix3x3 orientation = Matrix3x3(q);

	mInverseInteriaTensor = orientation * Matrix3x3::Scale(mInverseInertia) * invOrientation;
}