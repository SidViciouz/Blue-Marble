#include "Transform.h"
#include "Matrix4x4.h"
#include "Matrix3x3.h"

using namespace Maths;

Transform::Transform()
{
	scale = Vector3(1, 1, 1);
}

void Transform::UpdateMatrix() {
	matrix =
		Matrix4x4::Translation(position) *
		Matrix4x4(quaternion) *
		Matrix4x4::Scale(scale);
}

Transform& Transform::SetPosition(const Vector3& worldPos) {
	position = worldPos;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetScale(const Vector3& worldScale) {
	scale = worldScale;
	UpdateMatrix();
	return *this;
}

Transform& Transform::SetOrientation(const Quaternion& worldOrientation) {
	quaternion = worldOrientation;
	UpdateMatrix();
	return *this;
}

Vector3 Transform::GetPosition() const
{
	return position;
}

Vector3 Transform::GetScale() const
{
	return scale;
}

Quaternion Transform::GetOrientation() const
{
	return quaternion;
}

Matrix3x3 Transform::GetRotMatrix() const
{
	return Matrix3x3(quaternion);
}

Matrix3x3 Transform::GetInvRotMatrix() const
{
	return Matrix3x3(quaternion.Conjugate());
}