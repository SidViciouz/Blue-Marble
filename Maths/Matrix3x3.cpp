#include "Matrix3x3.h"
#include "Vector3.h"
#include "Quaternion.h"

using namespace Maths;

Matrix3x3::Matrix3x3(void)
{
	for (int i = 0; i < 9; ++i) {
		array[i] = 0.0f;
	}
	array[0] = 1.0f;
	array[4] = 1.0f;
	array[8] = 1.0f;
}

Matrix3x3::Matrix3x3(float elements[9])
{
	array[0] = elements[0];
	array[1] = elements[1];
	array[2] = elements[2];

	array[3] = elements[4];
	array[4] = elements[5];
	array[5] = elements[6];

	array[6] = elements[8];
	array[7] = elements[9];
	array[8] = elements[10];
}

Matrix3x3::Matrix3x3(const Quaternion& quat)
{

	XMFLOAT4 q = quat.Get();

	float yy = q.y * q.y;
	float zz = q.z * q.z;
	float xy = q.x * q.y;
	float zw = q.z * q.w;
	float xz = q.x * q.z;
	float yw = q.y * q.w;
	float xx = q.x * q.x;
	float yz = q.y * q.z;
	float xw = q.x * q.w;

	array[0] = 1 - 2 * yy - 2 * zz;
	array[1] = 2 * xy + 2 * zw;
	array[2] = 2 * xz - 2 * yw;

	array[3] = 2 * xy - 2 * zw;
	array[4] = 1 - 2 * xx - 2 * zz;
	array[5] = 2 * yz + 2 * xw;

	array[6] = 2 * xz + 2 * yw;
	array[7] = 2 * yz - 2 * xw;
	array[8] = 1 - 2 * xx - 2 * yy;
}


Matrix3x3::~Matrix3x3(void)
{

}

Matrix3x3 Matrix3x3::Rotation(float degrees, const Vector3& inaxis)
{
	Matrix3x3 m;

	Vector3 axis = inaxis;

	axis.Normalize();
	
	float c = cos(XMConvertToRadians(degrees));
	float s = sin(XMConvertToRadians(degrees));

	m.array[0] = (axis.v.x * axis.v.x) * (1.0f - c) + c;
	m.array[1] = (axis.v.y * axis.v.x) * (1.0f - c) + (axis.v.z * s);
	m.array[2] = (axis.v.z * axis.v.x) * (1.0f - c) - (axis.v.y * s);

	m.array[3] = (axis.v.x * axis.v.y) * (1.0f - c) - (axis.v.z * s);
	m.array[4] = (axis.v.y * axis.v.y) * (1.0f - c) + c;
	m.array[5] = (axis.v.z * axis.v.y) * (1.0f - c) + (axis.v.x * s);

	m.array[6] = (axis.v.x * axis.v.z) * (1.0f - c) + (axis.v.y * s);
	m.array[7] = (axis.v.y * axis.v.z) * (1.0f - c) - (axis.v.x * s);
	m.array[8] = (axis.v.z * axis.v.z) * (1.0f - c) + c;

	return m;
}

Matrix3x3 Matrix3x3::Scale(const Vector3& scale)
{
	Matrix3x3 m;

	m.array[0] = scale.v.x;
	m.array[4] = scale.v.y;
	m.array[8] = scale.v.z;

	return m;
}

void Matrix3x3::ToZero()
{
	for (int i = 0; i < 9; ++i) {
		array[0] = 0.0f;
	}
}

Vector3 Matrix3x3::ToEuler() const
{

	float testVal = abs(array[2]) + 0.00001f;

	if (testVal < 1.0f) {
		float theta1 = -asin(array[2]);
		float theta2 = XM_PI - theta1;

		float cost1 = cos(theta1);
		//float cost2 = cos(theta2);

		float psi1 = XMConvertToDegrees(atan2(array[5] / cost1, array[8] / cost1));
		//float psi2 = Maths::RadiansToDegrees(atan2(array[5] / cost2, array[8] / cost2));

		float phi1 = XMConvertToDegrees(atan2(array[1] / cost1, array[0] / cost1));
		//float phi2 = Maths::RadiansToDegrees(atan2(array[1] / cost2, array[0] / cost2));

		theta1 = XMConvertToDegrees(theta1);
		//theta2 = Maths::RadiansToDegrees(theta2);

		return Vector3(psi1, theta1, phi1);
	}
	else {
		float phi = 0.0f;	//x


		float theta = 0.0f;	//y
		float psi = 0.0f;	//z

		float delta = atan2(array[3], array[6]);

		if (array[2] < 0.0f) {
			theta = XM_PI / 2.0f;
			psi = phi + delta;
		}
		else {
			theta = -XM_PI / 2.0f;
			psi = phi + delta;
		}

		return Vector3(XMConvertToDegrees(psi), XMConvertToDegrees(theta), XMConvertToDegrees(phi));
	}

}

Matrix3x3 Matrix3x3::FromEuler(const Vector3& euler)
{
	Matrix3x3 m;
	
	float heading = XMConvertToRadians(euler.v.y);
	float attitude = XMConvertToRadians(euler.v.x);
	float bank = XMConvertToRadians(euler.v.z);

	float ch = cos(heading);
	float sh = sin(heading);
	float ca = cos(attitude);
	float sa = sin(attitude);
	float cb = cos(bank);
	float sb = sin(bank);

	m.array[0] = ch * ca;
	m.array[3] = sh * sb - ch * sa * cb;
	m.array[6] = ch * sa * sb + sh * cb;
	m.array[1] = sa;
	m.array[4] = ca * cb;
	m.array[7] = -ca * sb;
	m.array[2] = -sh * ca;
	m.array[5] = sh * sa * cb + ch * sb;
	m.array[8] = -sh * sa * sb + ch * cb;

	return m;
}

Vector3 Matrix3x3::GetRow(unsigned int row) const
{
	assert(row < 3);
	int start = row;
	return Vector3(
		array[start],
		array[start + 3],
		array[start + 6]
	);
}

void Matrix3x3::SetRow(unsigned int row, const Vector3& val)
{
	assert(row < 3);

	int start = 3 * row;

	array[start] = val.v.x;
	array[start + 3] = val.v.y;
	array[start + 6] = val.v.z;
}

Vector3 Matrix3x3::GetColumn(unsigned int column) const
{
	assert(column < 3);
	int start = 3 * column;
	return Vector3(
		array[start],
		array[start + 1],
		array[start + 2]
	);
}

void Matrix3x3::SetColumn(unsigned int column, const Vector3& val)
{
	assert(column < 3);
	int start = 3 * column;
	array[start] = val.v.x;
	array[start + 1] = val.v.y;
	array[start + 2] = val.v.z;
}

Vector3 Matrix3x3::GetDiagonal() const
{
	return Vector3(array[0], array[4], array[8]);
}

void Matrix3x3::SetDiagonal(const Vector3& in)
{
	array[0] = in.v.x;
	array[4] = in.v.y;
	array[8] = in.v.z;
}

Vector3 Matrix3x3::operator*(const Vector3& v) const
{
	Vector3 vec;

	vec.v.x = v.v.x * array[0] + v.v.y * array[3] + v.v.z * array[6];
	vec.v.y = v.v.x * array[1] + v.v.y * array[4] + v.v.z * array[7];
	vec.v.z = v.v.x * array[2] + v.v.y * array[5] + v.v.z * array[8];

	return vec;
};