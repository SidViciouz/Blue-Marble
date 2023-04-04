#include "RigidBodyComponent.h"
#include "SceneNode.h"
#include "BoxCollisionComponent.h"

RigidBodyComponent::RigidBodyComponent(shared_ptr<SceneNode> NodeAttachedTo,float mass)
	: mNodeAttachedTo{ NodeAttachedTo }, mMass{ mass }
{
	BoxCollisionComponent* box = dynamic_cast<BoxCollisionComponent*>(mNodeAttachedTo->mCollisionComponent.get());
	
	float w = box->mWidth;
	float h = box->mHeight;
	float d = box->mDepth;

	mInertiaTensor = 
	{
		mMass*(h*h + d*d)/12.0f, 0, 0,
		0, mMass*(w*w + d*d)/12.0f,0,
		0,0,mMass*(w*w+h*h)/12.0f
	};

	XMMATRIX inertiaM = XMLoadFloat3x3(&mInertiaTensor);
	XMVECTOR det = XMMatrixDeterminant(inertiaM);
	XMStoreFloat3x3(&mInvInertiaTensor, XMMatrixInverse(&det, inertiaM));

	mPosition.v = mNodeAttachedTo->mAccumulatedPosition.Get();
	mRotation = mNodeAttachedTo->mAccumulatedQuaternion;
}

void RigidBodyComponent::Update(float deltaTime)
{
	// update linear properties
	mPosition.v = mNodeAttachedTo->mAccumulatedPosition.Get();

	Vector3 deltaVel = mForce * (deltaTime / mMass);

	mVelocity = mVelocity + deltaVel;

	mPosition = mPosition + (mVelocity * deltaTime);

	// update angular properties
	mRotation = mNodeAttachedTo->mAccumulatedQuaternion;

	XMMATRIX rotationM = XMMatrixRotationQuaternion(XMLoadFloat4(&mRotation.Get()));
	XMMATRIX rotationMT = XMMatrixTranspose(rotationM);
	XMMATRIX invInertiaM = XMLoadFloat3x3(&mInvInertiaTensor);

	XMStoreFloat3x3(&mRotatedInvInertiaTensor,XMMatrixMultiply(XMMatrixMultiply(rotationM,invInertiaM),rotationMT));

	Vector3 deltaAngularVel = (mTorque * deltaTime) * mRotatedInvInertiaTensor;

	mAngularVel = mAngularVel + deltaAngularVel;

	float deltaAngle = (mAngularVel * deltaTime).length() / 2.0f;

	Vector3 vec = mAngularVel.normalize() * sinf(deltaAngle);
	Quaternion deltaQuat( vec.v.x, vec.v.y, vec.v.z, cosf(deltaAngle));

	mRotation.Mul(deltaQuat);

	mForce = Vector3();
	mTorque = Vector3();

	mNodeAttachedTo->mRelativePosition.Add((mVelocity * deltaTime).v);
	mNodeAttachedTo->mRelativeQuaternion.Set(mRotation);
}

void RigidBodyComponent::AddForce(Vector3 force, Vector3 relativePosition)
{
	mForce = mForce + force;
	mTorque = mTorque + relativePosition ^ force;
}

void RigidBodyComponent::AddImpulse(CollisionInfo& collisionInfo, shared_ptr<RigidBodyComponent> other,float deltaTime)
{
	XMFLOAT3 xmP1 = this->mNodeAttachedTo->mAccumulatedPosition.Get();
	Vector3 p1(xmP1.x, xmP1.y, xmP1.z);
	XMFLOAT3 xmP2 = other->mNodeAttachedTo->mAccumulatedPosition.Get();
	Vector3 p2(xmP2.x, xmP2.y, xmP2.z);

	float e = 0.3f;
	Vector3 n = collisionInfo.normal;
	float invM1 = 1.0f/this->mMass;
	float invM2 = 1.0f/other->mMass;
	XMFLOAT3X3 I1 = this->mRotatedInvInertiaTensor;
	XMFLOAT3X3 I2 = other->mRotatedInvInertiaTensor;
	Vector3 r1 = collisionInfo.localA - p1;
	Vector3 r2 = collisionInfo.localB - p2;
	Vector3 vr = (this->mVelocity + (this->mAngularVel^r1)) - (other->mVelocity + (other->mAngularVel ^ r2));
	
	float denominator = invM1 + invM2 + ((( (r1 ^ n) * I1 )^ r1) + (((r2 ^ n) * I2 )^ r2)) * n;
	float impulse =  (1 + e)*(vr*n)/denominator;

	AddForce(n*impulse/deltaTime,r1);
}
