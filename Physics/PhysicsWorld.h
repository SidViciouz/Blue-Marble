#pragma once

#include "PhysicsObject.h"
#include <vector>
#include <functional>

namespace Physics
{
	using PhysicsObjectFunc = std::function<void(PhysicsObject*)>;

	class PhysicsWorld {
	public:
												PhysicsWorld();
												~PhysicsWorld() {};

		void									ClearForces();

		void									AddPhysicsObject(PhysicsObject* object);

		void									UpdateVelocity(float deltaTime);

		void									ApplyForce(float deltaTime);

		void									GetObjectIterators(std::vector<PhysicsObject*>::const_iterator& first, std::vector<PhysicsObject*>::const_iterator& last) const;

	protected:
		std::vector<PhysicsObject*>				mGameObjects;
	};
}
