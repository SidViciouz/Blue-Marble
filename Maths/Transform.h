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

	protected:
		Vector3									position;
		Quaternion								quaternion;
		Vector3									scale;
	};
}