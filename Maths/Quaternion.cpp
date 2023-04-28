#include "Quaternion.h"

using namespace Maths;

Quaternion::Quaternion()
{
	Set(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion::Quaternion(const float& x, const float& y, const float& z, const float& w)
{
	Set(x, y, z, w);
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

const XMFLOAT4& Quaternion::Get() const
{
	return mQuaternion;
}
