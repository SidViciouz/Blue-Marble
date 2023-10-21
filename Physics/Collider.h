#pragma once

#include "../Maths/Vector3.h"
#include "../Maths/Transform.h"
#include "../Maths/Matrix3x3.h"

using namespace Maths;

namespace Physics
{

	class Collider
	{
	public:
												Collider() {}
												~Collider() {}

		virtual Vector3							Support(const Vector3& dir, const Transform& transform) = 0;

	};

	class AABBCollider : Collider
	{

	public:
		AABBCollider(const Vector3& halfSize);
		~AABBCollider() {}

		Vector3 Support(const Vector3& direction, const Transform& transform);

	protected:
		Vector3 halfSize;
	};
}