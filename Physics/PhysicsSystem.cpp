#include "PhysicsSystem.h"
#include "PhysicsObject.h"
#include "PhysicsWorld.h"

#include <functional>
#include <vector>

using namespace Physics;

PhysicsSystem::PhysicsSystem(shared_ptr<PhysicsWorld> physicsWorld) :
	physicsWorld(physicsWorld)
{
	applyGravity = false;
	useBroadPhase = false;
	dTOffset = 0.0f;
	globalDamping = 0.995f;

	staticCountMax = 25;
	staticMaxPosMagn = 0.0004;

	SetGravity(Vector3(0.0f, -9.8f, 0.0f));

	bPhysics = true;
}

PhysicsSystem::~PhysicsSystem()
{

}

void PhysicsSystem::SetGravity(const Vector3& g)
{
	gravity = g;
}

void PhysicsSystem::Clear()
{
	allBroadPhaseCollisions.clear();
}

const int   idealHZ = 120;
const float idealDT = 1.0f / idealHZ;

int realHZ = idealHZ;
float realDT = idealDT;

void PhysicsSystem::Update(float dt)
{
	dTOffset += dt;

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
	std::vector < PhysicsObject* >::const_iterator first;
	std::vector < PhysicsObject* >::const_iterator last;
	physicsWorld->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i)
	{

		for (auto j = i + 1; j != last; ++j)
		{

			if ((*i)->GetPhysicsType() == PhysicsType::Static && (*j)->GetPhysicsType() == PhysicsType::Static) {
				continue;
			}

			CollisionInfo info;

			if (GJKCalculation(*i, *j, info))
			{
				if (bPhysics)
				{
					ImpulseResolveCollision(*info.a, *info.b, info.point);
				}

				info.framesLeft = numCollisionFrames;
				allBroadPhaseCollisions.insert(info);
			}
		}
	}
}

void PhysicsSystem::ImpulseResolveCollision(PhysicsObject& a, PhysicsObject& b, ContactPoint& p) const
{

	PhysicsObject* physA = &a;
	PhysicsObject* physB = &b;

	Transform& transformA = a.GetTransform();
	Transform& transformB = b.GetTransform();

	float totalMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (totalMass == 0)
	{
		return;
	}

	// Separate them out using projection
	transformA.SetPosition(transformA.GetPosition() -
		(p.normal * p.penetration * (physA->GetInverseMass() / totalMass)));

	transformB.SetPosition(transformB.GetPosition() +
		(p.normal * p.penetration * (physB->GetInverseMass() / totalMass)));


	Vector3 relativeA = p.localA;
	Vector3 relativeB = p.localB;

	Vector3 angVelocityA =
		Vector3::Cross(physA->GetAngularVelocity(), relativeA);
	Vector3 angVelocityB =
		Vector3::Cross(physB->GetAngularVelocity(), relativeB);

	Vector3 fullVelocityA = physA->GetLinearVelocity() + angVelocityA;
	Vector3 fullVelocityB = physB->GetLinearVelocity() + angVelocityB;

	Vector3 contactVelocity = fullVelocityB - fullVelocityA;

	float impulseForce = Vector3::Dot(contactVelocity, p.normal);

	// now to work out the effect of inertia ....
	Vector3 inertiaA = Vector3::Cross(physA->GetInertiaTensor() *
		Vector3::Cross(relativeA, p.normal), relativeA); //?
	Vector3 inertiaB = Vector3::Cross(physB->GetInertiaTensor() *
		Vector3::Cross(relativeB, p.normal), relativeB);
	float angularEffect = Vector3::Dot(inertiaA + inertiaB, p.normal);

	//float cRestitution = 0.66f; // disperse some kinetic energy
	float cRestitution = physA->GetElasticity() * physB->GetElasticity();

	float j = (-(1.0f + cRestitution) * impulseForce) /
		(totalMass + angularEffect);

	Vector3 fullImpulse = p.normal * j;


	physA->ApplyLinearImpulse(-fullImpulse);
	physB->ApplyLinearImpulse(fullImpulse);

	physA->ApplyAngularImpulse(Vector3::Cross(relativeA, -fullImpulse));
	physB->ApplyAngularImpulse(Vector3::Cross(relativeB, fullImpulse));
}


void PhysicsSystem::ApplyForce(float dt) {
	std::vector < PhysicsObject* >::const_iterator first;
	std::vector < PhysicsObject* >::const_iterator last;
	physicsWorld->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i)
	{
		PhysicsObject* object = (*i);

		//linear 값 계산
		float inverseMass = object->GetInverseMass();

		Vector3 linearVel = object->GetLinearVelocity();
		Vector3 force = object->GetForce();
		Vector3 accel = force * inverseMass;

		if (accel.Length() > 10 && inverseMass > 0)
		{
			object->staticPositionCount = 0;
		}

		if (applyGravity && inverseMass > 0)
		{
			accel += gravity;
		}

		linearVel += accel * dt;
		object->SetLinearVelocity(linearVel);

		// angular 값 계산
		Vector3 torque = object->GetTorque();
		Vector3 angVel = object->GetAngularVelocity();

		object->UpdateInertiaTensor();

		Vector3 angAccel = object->GetInertiaTensor() * torque;

		angVel += angAccel * dt;
		object->SetAngularVelocity(angVel);

	}
}

void PhysicsSystem::UpdateVelocity(float dt) {

	std::vector < PhysicsObject* >::const_iterator first;
	std::vector < PhysicsObject* >::const_iterator last;
	physicsWorld->GetObjectIterators(first, last);
	float frameLinearDamping = 1.0f - (0.4f * dt);

	for (auto i = first; i != last; ++i)
	{
		PhysicsObject* object = (*i);

		Transform& transform = (*i)->GetTransform();
		// Position Stuff
		Vector3 position = transform.GetPosition();
		Vector3 linearVel = object->GetLinearVelocity();

		Vector3 dPosition = linearVel * dt;
		float mag_position = (linearVel * dt).Length();

		if (applyGravity)
		{
			if (dPosition.Length() < staticMaxPosMagn)
			{
				object->staticPositionCount++;
			}
		}

		if (object->staticPositionCount < staticCountMax)
		{

			position += linearVel * dt;

			transform.SetPosition(position);
			// Linear Damping
			linearVel = linearVel * frameLinearDamping; //why after?
			object->SetLinearVelocity(linearVel);


			// Orientation Stuff
			Quaternion orientation = transform.GetOrientation();
			Vector3 angVel = object->GetAngularVelocity();

			if ((*i)->GetPhysicsType() == PhysicsType::Pawn) {
				angVel = Vector3(0, 1, 0) * Vector3::Dot(angVel, Vector3(0, 1, 0));
			}
			//if (angVel.Length() > 0.08) {
			Vector3 dAngle = angVel * dt;
			float mag_dAngle = dAngle.Length();

			orientation = orientation +
				(Quaternion(angVel * dt * 0.5f, 0.0f) * orientation);
			orientation.Normalise();



			transform.SetOrientation(orientation);

			// Damp the angular velocity too
			float frameAngularDamping = 1.0f - (0.4f * dt);
			angVel = angVel * frameAngularDamping;
			object->SetAngularVelocity(angVel);

		}
		else
		{
			object->staticPositionCount = staticCountMax;
			object->SetLinearVelocity(Vector3(0, 0, 0));
		}

	}

}

void PhysicsSystem::ClearForces()
{
	physicsWorld->OperateOnContents(
		[](PhysicsObject* o) {
		o->ClearForces();
	}
	);
}