#include "Quaternion.h"
#include "Vector3.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "Maths.h"
#include <algorithm>

using namespace Maths;


Quaternion::Quaternion(void)
{
	x = y = z = 0.0f;
	w = 1.0f;
}

Quaternion::Quaternion(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

Quaternion::Quaternion(const Vector3& vector, float w) {
	this->x = vector.v.x;
	this->y = vector.v.y;
	this->z = vector.v.z;
	this->w = w;
}

Quaternion::Quaternion(const Matrix4x4& m) {
	w = sqrt(std::max(0.0f, (1.0f + m.array[0] + m.array[5] + m.array[10]))) * 0.5f;

	if (abs(w) < 0.0001f) {
		x = sqrt(std::max(0.0f, (1.0f + m.array[0] - m.array[5] - m.array[10]))) / 2.0f;
		y = sqrt(std::max(0.0f, (1.0f - m.array[0] + m.array[5] - m.array[10]))) / 2.0f;
		z = sqrt(std::max(0.0f, (1.0f - m.array[0] - m.array[5] + m.array[10]))) / 2.0f;

		x = (float)copysign(x, m.array[9] - m.array[6]);
		y = (float)copysign(y, m.array[2] - m.array[8]);
		z = (float)copysign(z, m.array[4] - m.array[1]);
	}
	else {
		float qrFour = 4.0f * w;
		float qrFourRecip = 1.0f / qrFour;

		x = (m.array[6] - m.array[9]) * qrFourRecip;
		y = (m.array[8] - m.array[2]) * qrFourRecip;
		z = (m.array[1] - m.array[4]) * qrFourRecip;
	}
}

Quaternion::Quaternion(const Matrix3x3& m) {
	w = sqrt(std::max(0.0f, (1.0f + m.array[0] + m.array[4] + m.array[8]))) * 0.5f;

	float qrFour = 4.0f * w;
	float qrFourRecip = 1.0f / qrFour;

	x = (m.array[5] - m.array[7]) * qrFourRecip;
	y = (m.array[6] - m.array[2]) * qrFourRecip;
	z = (m.array[1] - m.array[3]) * qrFourRecip;
}

Quaternion::~Quaternion(void)
{
}

float Quaternion::Dot(const Quaternion& a, const Quaternion& b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

void Quaternion::Normalise() {
	float magnitude = sqrt(x * x + y * y + z * z + w * w);

	if (magnitude > 0.0f) {
		float t = 1.0f / magnitude;

		x *= t;
		y *= t;
		z *= t;
		w *= t;
	}
}

XMFLOAT4 Quaternion::Get() const
{
	return XMFLOAT4(x, y, z, w);
}

void Quaternion::CalculateW() {
	w = 1.0f - (x * x) - (y * y) - (z * z);
	if (w < 0.0f) {
		w = 0.0f;
	}
	else {
		w = -sqrt(w);
	}
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Lerp(const Quaternion& from, const Quaternion& to, float by) {
	Quaternion temp = to;

	float dot = Quaternion::Dot(from, to);

	if (dot < 0.0f) {
		temp = -to;
	}

	return (from * (1.0f - by)) + (temp * by);
}

Quaternion Quaternion::Slerp(const Quaternion& from, const Quaternion& to, float by) {
	Quaternion temp = to;

	float dot = Quaternion::Dot(from, to);

	if (dot < 0.0f) {
		temp = -to;
	}

	return (from * (cos(by))) + (to * (1.0f - cos(by)));
}

//http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
//Verified! Different values to above, due to difference between x/z being 'forward'
Vector3 Quaternion::ToEuler() const {
	Vector3 euler;

	float t = x * y + z * w;

	if (t > 0.4999) {
		euler.v.z = Maths::RadiansToDegrees(Maths::PI / 2.0f);
		euler.v.y = Maths::RadiansToDegrees(2.0f * atan2(x, w));
		euler.v.x = 0.0f;

		return euler;
	}

	if (t < -0.4999) {
		euler.v.z = -Maths::RadiansToDegrees(Maths::PI / 2.0f);
		euler.v.y = -Maths::RadiansToDegrees(2.0f * atan2(x, w));
		euler.v.x = 0.0f;
		return euler;
	}

	float sqx = x * x;
	float sqy = y * y;
	float sqz = z * z;

	euler.v.z = Maths::RadiansToDegrees(asin(2 * t));
	euler.v.y = Maths::RadiansToDegrees(atan2(2 * y * w - 2 * x * z, 1.0f - 2 * sqy - 2 * sqz));
	euler.v.x = Maths::RadiansToDegrees(atan2(2 * x * w - 2 * y * z, 1.0f - 2 * sqx - 2.0f * sqz));

	return euler;
}

//http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/
//VERIFIED AS CORRECT - Pitch and roll are changed around as the above uses x as 'forward', whereas we use -z
Quaternion Quaternion::EulerAnglesToQuaternion(float roll, float yaw, float pitch) {
	float cos1 = (float)cos(Maths::DegreesToRadians(yaw * 0.5f));
	float cos2 = (float)cos(Maths::DegreesToRadians(pitch * 0.5f));
	float cos3 = (float)cos(Maths::DegreesToRadians(roll * 0.5f));

	float sin1 = (float)sin(Maths::DegreesToRadians(yaw * 0.5f));
	float sin2 = (float)sin(Maths::DegreesToRadians(pitch * 0.5f));
	float sin3 = (float)sin(Maths::DegreesToRadians(roll * 0.5f));

	Quaternion q;

	q.x = (sin1 * sin2 * cos3) + (cos1 * cos2 * sin3);
	q.y = (sin1 * cos2 * cos3) + (cos1 * sin2 * sin3);
	q.z = (cos1 * sin2 * cos3) - (sin1 * cos2 * sin3);
	q.w = (cos1 * cos2 * cos3) - (sin1 * sin2 * sin3);

	return q;
};

Quaternion Quaternion::AxisAngleToQuaterion(const Vector3& vector, float degrees) {
	float theta = (float)Maths::DegreesToRadians(degrees);
	float result = (float)sin(theta / 2.0f);

	return Quaternion((float)(vector.v.x * result), (float)(vector.v.y * result), (float)(vector.v.z * result), (float)cos(theta / 2.0f));
}


Vector3		Quaternion::operator *(const Vector3& a)	const {
	Quaternion newVec = *this * Quaternion(a.v.x, a.v.y, a.v.z, 0.0f) * Conjugate();
	return Vector3(newVec.x, newVec.y, newVec.z);
}
/*
Quaternion::Quaternion()
{
	Set(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion::Quaternion(const float& x, const float& y, const float& z, const float& w)
{
	Set(x, y, z, w);
}

Quaternion::Quaternion(const Vector3& vector, float w) {
	mQuaternion.x = vector.v.x;
	mQuaternion.y = vector.v.y;
	mQuaternion.z = vector.v.z;
	mQuaternion.w = w;
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion quaternion = *this;

	quaternion *= other.mQuaternion;

	return quaternion;
}

void Quaternion::Set(const XMFLOAT4& quaternion)
{
	XMStoreFloat4(&mQuaternion, XMQuaternionNormalize(XMLoadFloat4(&quaternion)));
}

void Quaternion::Set(const Quaternion& quaternion)
{
	XMStoreFloat4(&mQuaternion, XMQuaternionNormalize(XMLoadFloat4(&quaternion.mQuaternion)));
}

void Quaternion::Set(const float& x, const float& y, const float& z, const float& w)
{
	XMStoreFloat4(&mQuaternion, XMQuaternionNormalize(XMVectorSet(x, y, z, w)));
}

void Quaternion::operator*=(const XMFLOAT4& quaternion)
{
	XMStoreFloat4(&mQuaternion, XMQuaternionMultiply(XMLoadFloat4(&mQuaternion), XMQuaternionNormalize(XMLoadFloat4(&quaternion))));
}

void Quaternion::operator*=(const Quaternion& quaternion)
{
	XMStoreFloat4(&mQuaternion, XMQuaternionMultiply(XMLoadFloat4(&mQuaternion), XMQuaternionNormalize(XMLoadFloat4(&quaternion.mQuaternion))));
}

void Quaternion::Mul(const float& x, const float& y, const float& z, const float& w)
{
	XMStoreFloat4(&mQuaternion, XMQuaternionMultiply(XMLoadFloat4(&mQuaternion), XMQuaternionNormalize(XMVectorSet(x, y, z, w))));
}

Quaternion Quaternion::operator+(const Quaternion& other) const
{
	return Quaternion(mQuaternion.x + other.mQuaternion.x,
		mQuaternion.y + other.mQuaternion.y,
		mQuaternion.z + other.mQuaternion.z,
		mQuaternion.w + other.mQuaternion.w);
}

void Quaternion::Normalize()
{
	XMStoreFloat4(&mQuaternion, XMVector4Normalize(XMLoadFloat4(&mQuaternion)));
}

Quaternion Quaternion::Conjugate() const
{
	return Quaternion(-mQuaternion.x, -mQuaternion.y, -mQuaternion.z, mQuaternion.w);
}

const XMFLOAT4& Quaternion::Get() const
{
	return mQuaternion;
}
*/