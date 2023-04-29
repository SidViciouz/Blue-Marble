#include "Quaternion.h"
#include "Vector3.h"

using namespace Maths;

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
