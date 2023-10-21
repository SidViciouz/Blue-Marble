#include "PhysicsWorld.h"
#include <random>

using namespace Physics;

PhysicsWorld::PhysicsWorld()
{
}

void PhysicsWorld::ClearForces()
{
	for (auto& object : mGameObjects)
	{
		object->ClearForces();
	}
}

void PhysicsWorld::AddPhysicsObject(PhysicsObject* object)
{
	mGameObjects.emplace_back(object);
}


void PhysicsWorld::UpdateVelocity(float deltaTime)
{
	float damping = 0.995f;

	for (auto it = mGameObjects.begin(); it != mGameObjects.end(); ++it)
	{
		Transform& transform = (*it)->GetTransform();

		// linear update
		Vector3 position = transform.GetPosition();
		Vector3 linearVelocity = (*it)->GetLinearVelocity();

		position += linearVelocity * deltaTime;
		transform.SetPosition(position);

		// angular update
		Quaternion orientation = transform.GetOrientation();
		Vector3 angularVelocity = (*it)->GetAngularVelocity();

		orientation = orientation + Quaternion(angularVelocity * deltaTime * 0.5f, 0.0f) * orientation;
		orientation.Normalise();

		transform.SetOrientation(orientation);

		// Linear Damping
		linearVelocity = linearVelocity * damping;

		// angular damping
		angularVelocity = angularVelocity * damping;

		(*it)->SetLinearVelocity(linearVelocity);

		(*it)->SetAngularVelocity(angularVelocity);
	}
}

void PhysicsWorld::ApplyForce(float deltaTime)
{
	for (auto it = mGameObjects.begin(); it != mGameObjects.end(); ++it)
	{
		PhysicsObject* object = (*it);

		//linear 값 계산
		float inverseMass = object->GetInverseMass();

		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;

		if (inverseMass > 0)
		{
			accel += Vector3(0.0f,-9.8f,0.0f);
		}

		linearVel += accel * deltaTime;
		object->SetLinearVelocity(linearVel);

		// angular 값 계산
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();

		object->UpdateInertiaTensor();

		Vector3 angAccel = object->GetInertiaTensor() * torque;

		angVel += angAccel * deltaTime;
		object->SetAngularVelocity(angVel);
	}
}


void PhysicsWorld::GetObjectIterators(std::vector<PhysicsObject*>::const_iterator& first, std::vector<PhysicsObject*>::const_iterator& last) const
{
	first = mGameObjects.begin();
	last = mGameObjects.end();
}