#pragma once

#include "../Maths/Transform.h"
#include "Collider.h"
#include <memory>

using namespace Maths;
using namespace std;

namespace Physics
{
	class PhysicsObject
	{
	public:
												PhysicsObject();
												~PhysicsObject() {}

		void									SetCollider(shared_ptr<Collider> collider);
		shared_ptr<Collider>					GetCollider() const;

		Transform&								GetTransform();

	protected:
		Transform								mTransform;

		shared_ptr<Collider>					mCollider;
	};
}

