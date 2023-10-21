#pragma once

#include "PhysicsObject.h"
#include "Collision.h"
#include "PhysicsWorld.h"
#include <set>
#include "../Maths/Quaternion.h"

using namespace Maths;

namespace Physics
{
	class PhysicsSystem
	{
	public:
												PhysicsSystem(shared_ptr<PhysicsWorld> physicsWorld);
												~PhysicsSystem() {};

		void									Update(float deltaTime);

	protected:

		void									CollisionDetection();

		void									ClearForces();

		void									ApplyForce(float deltaTime);

		void									UpdateVelocity(float deltaTime);

		void									ResolveCollisionByImpulse(PhysicsObject& a, PhysicsObject& b, ContactInfo& p) const;

		shared_ptr<PhysicsWorld>				physicsWorld;

		Vector3									gravity;

		float									dTOffset;

		float									realDT;
	};
}
