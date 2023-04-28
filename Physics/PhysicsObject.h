#pragma once

#include "../Maths/Transform.h"
#include "Collider.h"
#include <memory>

using namespace Maths;
using namespace std;

namespace Physics
{
	class PhyscisObject
	{
	public:
												PhyscisObject();
												~PhyscisObject() {}

		void									SetCollider(shared_ptr<Collider> collider);
	protected:
		Transform								mTransform;

		shared_ptr<Collider>					mCollider;
	};
}

