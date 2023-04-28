#include "Transform.h"

using namespace Maths;

Transform::Transform()
{
	scale = Vector3(1, 1, 1);
}

Vector3 Transform::GetPosition() const
{
	return position;
}