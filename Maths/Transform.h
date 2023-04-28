#pragma once

#include "Vector3.h"
#include "Quaternion.h"

namespace Maths
{
	class Transform
	{
	public:
												Transform();
												~Transform() {}

		Vector3									GetPosition() const;

	protected:
		Vector3									position;
		Quaternion								quaternion;
		Vector3									scale;
	};
}