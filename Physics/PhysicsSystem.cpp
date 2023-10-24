#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "PhysicsWorld.h"

#include <functional>
#include <vector>

using namespace Physics;

PhysicsSystem::PhysicsSystem(shared_ptr<PhysicsWorld> physicsWorld) :
	physicsWorld(physicsWorld)
{
	dTOffset = 0.0f;
	realDT = 1.0f / 120.0f;
}


void PhysicsSystem::Update(float deltaTime)
{
	dTOffset += deltaTime;

	//fixed update
	while (dTOffset >= realDT)
	{
		ApplyForce(realDT);

		CollisionDetection();

		UpdateVelocity(realDT);

		dTOffset -= realDT;
	}

	ClearForces();
}


void PhysicsSystem::CollisionDetection()
{
	std::vector<PhysicsObject*>::const_iterator first;
	std::vector<PhysicsObject*>::const_iterator last;
	physicsWorld->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i)
	{
		for (auto j = i + 1; j != last; ++j)
		{
			if ((*i)->GetPhysicsType() == PhysicsType::Static && (*j)->GetPhysicsType() == PhysicsType::Static)
			{
				continue;
			}

			CollisionInfo info;

			info.a = *i;
			info.b = *j;

			if (GJK(info))
			{
				ResolveCollisionByImpulse(*info.a, *info.b, info.mInfo);
			}
		}
	}
}

void PhysicsSystem::ResolveCollisionByImpulse(PhysicsObject& a, PhysicsObject& b, ContactInfo& info) const
{
	PhysicsObject* A = &a;
	PhysicsObject* B = &b;

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	float MassSum = A->GetInverseMass() + B->GetInverseMass();

	if (MassSum == 0)
		return;

	transformA.SetPosition(transformA.GetPosition() - (info.normal * info.depth * (A->GetInverseMass() / MassSum)));
	transformB.SetPosition(transformB.GetPosition() + (info.normal * info.depth * (B->GetInverseMass() / MassSum)));

	Vector3 VelocityA = A->GetLinearVelocity() + Vector3::Cross(A->GetAngularVelocity(), info.a);
	Vector3 VelocityB = B->GetLinearVelocity() + Vector3::Cross(B->GetAngularVelocity(), info.b);

	float impulseForce = Vector3::Dot(VelocityB - VelocityA, info.normal);

	Vector3 Impulse = info.normal * impulseForce / MassSum;

	A->ApplyLinearImpulse(Impulse);
	B->ApplyLinearImpulse(-Impulse);

	A->ApplyAngularImpulse(Vector3::Cross(info.a, Impulse));
	B->ApplyAngularImpulse(Vector3::Cross(info.b, -Impulse));
}


void PhysicsSystem::ApplyForce(float deltaTime)
{
	physicsWorld->ApplyForce(deltaTime);
}

void PhysicsSystem::UpdateVelocity(float deltaTime)
{
	physicsWorld->UpdateVelocity(deltaTime);
}

void PhysicsSystem::ClearForces()
{
	physicsWorld->ClearForces();
}