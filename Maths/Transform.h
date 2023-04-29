#pragma once

#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"

namespace Maths
{
	class Transform
	{
	public:
												Transform();
												~Transform() {}

		void									UpdateMatrix();

		Transform&								SetPosition(const Vector3& worldPos);
		Transform&								SetScale(const Vector3& worldScale);
		Transform&								SetOrientation(const Quaternion& newOr);

		Vector3									GetPosition() const;
		Vector3									GetScale() const;
		Quaternion								GetOrientation() const;

	protected:

		Matrix4x4								matrix;

		Vector3									position;
		Quaternion								quaternion;
		Vector3									scale;
	};
}