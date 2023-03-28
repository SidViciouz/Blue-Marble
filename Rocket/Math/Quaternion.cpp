#include "Quaternion.h"

Quaternion::Quaternion()
{
	Set(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion quaternion = *this;

	quaternion.Mul(other.mQuaternion);

	return quaternion;
}

void Quaternion::Set(const XMFLOAT4& quaternion)
{	
	XMStoreFloat4(&mQuaternion,XMQuaternionNormalize(XMLoadFloat4(&quaternion)));
}

void Quaternion::Set(const float& x, const float& y, const float& z, const float& w)
{
	XMStoreFloat4(&mQuaternion, XMQuaternionNormalize(XMVectorSet(x,y,z,w)));
}

void Quaternion::Mul(const XMFLOAT4& quaternion)
{
	XMStoreFloat4(&mQuaternion, XMQuaternionMultiply(XMLoadFloat4(&mQuaternion), XMQuaternionNormalize(XMLoadFloat4(&quaternion))));
}

void Quaternion::Mul(const float& x, const float& y, const float& z, const float& w)
{
	XMStoreFloat4(&mQuaternion, XMQuaternionMultiply(XMLoadFloat4(&mQuaternion), XMQuaternionNormalize(XMVectorSet(x, y, z, w))));
}

const XMFLOAT4& Quaternion::Get()
{
	return mQuaternion;
}
