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
		Dynamic = 1
	};

	class PhysicsObject
	{
	public:
												PhysicsObject(PhysicsType type = PhysicsType::Dynamic);
												~PhysicsObject() {}

		void									SetCollider(Collider* collider);
		Collider*								GetCollider() const;

		Transform&								GetTransform();

		void									SetPhysicsType(PhysicsType type);

		PhysicsType								GetPhysicsType() const;

		void									SetInverseMass(float invMass);

		float									GetInverseMass() const;

		Vector3									GetLinearVelocity() const;

		Vector3									GetAngularVelocity() const;

		Vector3									GetTorque() const;

		Vector3									GetForce() const;

		void									ApplyAngularImpulse(const Vector3& force);

		void									ApplyLinearImpulse(const Vector3& force);

		void									AddForce(const Vector3& force);

		void									AddTorque(const Vector3& torque);

		void									ClearForces();

		void									SetLinearVelocity(const Vector3& velocity);

		void									SetAngularVelocity(const Vector3& velocity);

		void									InitializeCubeInertia();

		void									UpdateInertiaTensor();

		Matrix3x3								GetInertiaTensor() const;

	protected:

		Transform								mTransform;

		Collider*								mCollider;

		PhysicsType								mPhysicsType;

		float									mInverseMass;

		Vector3									mLinearVelocity;

		Vector3									mForce;

		Vector3									mAngularVelocity;

		Vector3									mTorque;

		Vector3									mInverseInertia;

		Matrix3x3								mInverseInteriaTensor;

	};
}

