#pragma once

#include "../Maths/Transform.h"
#include "Collider.h"
#include "../Maths/Matrix3x3.h"
#include <memory>

using namespace Maths;
using namespace std;

namespace Physics
{
	enum class PhysicsType
	{
		Static = 0,
		Dynamic = 1,
		Pawn = 2
	};

	class PhysicsObject
	{
	public:
												PhysicsObject(PhysicsType type = PhysicsType::Dynamic);
												~PhysicsObject() {}

		void									SetCollider(shared_ptr<Collider> collider);
		shared_ptr<Collider>					GetCollider() const;

		Transform&								GetTransform();

		PhysicsType								GetPhysicsType() const;

		void									SetWorldID(int newID);

		int										GetWorldID() const;

		void									SetInverseMass(float invMass);

		float									GetInverseMass() const;

		Vector3									GetLinearVelocity() const;

		Vector3									GetAngularVelocity() const;

		Vector3									GetTorque() const;

		Vector3									GetForce() const;

		void									ApplyAngularImpulse(const Vector3& force);
		void									ApplyLinearImpulse(const Vector3& force);

		void									AddForce(const Vector3& force);

		void									AddForceAtPosition(const Vector3& force, const Vector3& position);

		void									AddTorque(const Vector3& torque);


		void									ClearForces();

		void									SetLinearVelocity(const Vector3& v);

		void									SetAngularVelocity(const Vector3& v);

		void									InitCubeInertia();
		void									InitSphereInertia();

		void									UpdateInertiaTensor();

		Matrix3x3								GetInertiaTensor() const;

		/*test*/
		void									SetElasticity(float e);
		float									GetElasticity() const;

		int										staticPositionCount;

	protected:
		Transform								mTransform;

		shared_ptr<Collider>					mCollider;

		PhysicsType								mPhysicsType;

		int										mWorldID;

		float									mInverseMass;

		//linear stuff
		Vector3									mLinearVelocity;
		Vector3									mForce;


		//angular stuff
		Vector3									mAngularVelocity;
		Vector3									mTorque;
		Vector3									mInverseInertia;
		Matrix3x3								mInverseInteriaTensor;

		float									mElasticity;

	};
}

