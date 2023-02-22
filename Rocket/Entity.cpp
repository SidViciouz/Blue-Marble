#include "Entity.h"

void Entity::SetPosition(const XMFLOAT3& position)
{
	mPosition.Set(position);
	mDirty = true;
}

void Entity::SetPosition(const float& x, const float& y, const float& z)
{
	mPosition.Set(x, y, z);
	mDirty = true;
}

const XMFLOAT3& Entity::GetPosition()
{
	return mPosition.Get();
}

void Entity::AddPosition(const XMFLOAT3& d)
{
	mPosition.Add(d);
	mDirty = true;
}

void Entity::AddPosition(const float& x, const float& y, const float& z)
{
	mPosition.Add(x, y, z);
	mDirty = true;
}

void Entity::SetQuaternion(const XMFLOAT4& quaternion)
{
	mQuaternion.Set(quaternion);
	mDirty = true;
}

void Entity::SetQuaternion(const float& x, const float& y, const float& z, const float& w)
{
	mQuaternion.Set(x, y, z, w);
	mDirty = true;
}

const XMFLOAT4& Entity::GetQuaternion()
{
	return mQuaternion.Get();
}