#pragma once

#include "PhysicsObject.h"
#include "Constraint.h"
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

		void Clear();
		void ClearAndErase();

		void AddGameObject(PhysicsObject* o);
		void RemoveGameObject(PhysicsObject* o, bool andDelete = false);

		void AddConstraint(Constraint* c);
		void RemoveConstraint(Constraint* c, bool andDelete = false);

		void ShuffleConstraints(bool state) {
			shuffleConstraints = state;
		}

		void ShuffleObjects(bool state) {
			shuffleObjects = state;
		}

		virtual void UpdateWorld(float dt);

		void OperateOnContents(PhysicsObjectFunc f);

		void GetObjectIterators(
			PhysicsObjectIterator& first,
			PhysicsObjectIterator& last) const;

		void GetConstraintIterators(
			std::vector<Constraint*>::const_iterator& first,
			std::vector<Constraint*>::const_iterator& last) const;

	protected:
		std::vector<PhysicsObject*> gameObjects;
		std::vector<Constraint*> constraints;

		bool	shuffleConstraints;
		bool	shuffleObjects;
		int		worldIDCounter;
	};
}
