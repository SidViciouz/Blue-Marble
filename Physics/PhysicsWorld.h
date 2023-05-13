#pragma once

#include "PhysicsObject.h"
#include <vector>
#include <functional>

namespace Physics
{
	using PhysicsObjectFunc = std::function<void(PhysicsObject*)>;
	using PhysicsObjectIterator = std::vector<PhysicsObject*>::const_iterator;

	class PhysicsWorld {
	public:
												PhysicsWorld();
												~PhysicsWorld();

		void									Clear();
		void									ClearAndErase();

		void									AddGameObject(PhysicsObject* o);
		void									RemoveGameObject(PhysicsObject* o, bool andDelete = false);

		virtual void							UpdateWorld(float dt);

		void									OperateOnContents(PhysicsObjectFunc f);

		void									GetObjectIterators(
													PhysicsObjectIterator& first,
													PhysicsObjectIterator& last) const;

	protected:
		std::vector<PhysicsObject*>				gameObjects;

		int										worldIDCounter;
	};
}
