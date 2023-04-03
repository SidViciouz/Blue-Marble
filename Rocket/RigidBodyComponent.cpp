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

	mPosition = mPosition + (deltaVel * deltaTime);


	// update angular properties
	mRotation = mNodeAttachedTo->mAccumulatedQuaternion;

	XMMATRIX rotationM = XMMatrixRotationQuaternion(XMLoadFloat4(&mRotation.Get()));
	XMMATRIX rotationMT = XMMatrixTranspose(rotationM);
	XMMATRIX invInertiaM = XMLoadFloat3x3(&mInvInertiaTensor);

	XMFLOAT3X3 rotatedInvInertiaTensor;

	XMStoreFloat3x3(&rotatedInvInertiaTensor,XMMatrixMultiply(XMMatrixMultiply(rotationM,invInertiaM),rotationMT));

	Vector3 deltaAngularVel = (mTorque * deltaTime) * rotatedInvInertiaTensor;

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