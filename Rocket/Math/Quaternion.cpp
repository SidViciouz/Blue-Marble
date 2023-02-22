#include "Quaternion.h"

void Quaternion::Set(const XMFLOAT4& quaternion)
{
	mQuaternion = quaternion;
}

void Quaternion::Set(const float& x, const float& y, const float& z, const float& w)
{
	mQuaternion = { x,y,z,w };
}