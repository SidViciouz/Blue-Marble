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
												~PhysicsSystem();

		void									Clear();

		void									Update(float dt);

		bool bPhysics;

		void									UseGravity(bool state)
		{
			applyGravity = state;
		}

		void									SetGlobalDamping(float d)
		{
			globalDamping = d;
		}

		void									SetGravity(const Vector3& g);
	protected:
		void									CollisionDetection();

		void									ClearForces();

		void									ApplyForce(float dt);

		void									UpdateVelocity(float dt);

		void									ImpulseResolveCollision(PhysicsObject& a, PhysicsObject& b, ContactPoint& p) const;

		shared_ptr<PhysicsWorld>				physicsWorld;

		bool									applyGravity;
		Vector3									gravity;
		float									dTOffset;
		float									globalDamping;

		float									staticCountMax;
		float									staticMaxPosMagn;

		std::set<CollisionInfo>					allBroadPhaseCollisions;


		bool									useBroadPhase = true;
		int										numCollisionFrames = 5;
	};
}
