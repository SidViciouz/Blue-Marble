#include "Entity.h"

void Entity::SetPosition(const XMFLOAT3& position)
{
	mPosition.Set(position);
}

void Entity::SetPosition(const float& x, const float& y, const float& z)
{
	mPosition.Set(x, y, z);
}

const XMFLOAT3& Entity::GetPosition()
{
	return mPosition.Get();
}

void Entity::AddPosition(const XMFLOAT3& d)
{
	mPosition.Add(d);
}

void Entity::AddPosition(const float& x, const float& y, const float& z)
{
	mPosition.Add(x, y, z);
}

void Entity::SetQuaternion(const XMFLOAT4& quaternion)
{
	mQuaternion.Set(quaternion);
}

void Entity::SetQuaternion(const float& x, const float& y, const float& z, const float& w)
{
	mQuaternion.Set(x, y, z, w);
}

const XMFLOAT4& Entity::GetQuaternion()
{
	return mQuaternion.Get();
}