#pragma once

#include "../Maths/Vector3.h"
#include "../Maths/Transform.h"

using namespace Maths;

namespace Physics
{
	enum class ColliderType
	{
		None = 0,
		Sphere = 1,
		Capsule = 2,
		OBB = 3
	};

	class Collider
	{
	public:
												Collider();
												~Collider() {}

		virtual Vector3							Support(const Vector3& dir, const Transform& transform) = 0;

		const ColliderType&						GetType() const;
	protected:

		ColliderType							mColliderType;
	};
}