#include "Entity.h"

Entity::Entity()
{
	mPosition.Set(0.0f, 0.0f, 0.0f);
	mQuaternion.Set(0.0f, 0.0f, 0.0f,1.0f);
}

void Entity::Update()
{
	XMFLOAT3 pos = mPosition.Get();
	XMMATRIX world = XMMatrixRotationQuaternion(XMLoadFloat4(&mQuaternion.Get()))* XMMatrixTranslation(pos.x, pos.y, pos.z);

	XMStoreFloat4x4(&mObjFeature.world, world);

	mDirty = false;
}

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

void Entity::MulQuaternion(const XMFLOAT4& quaternion)
{
	mQuaternion.Mul(quaternion);
	mDirty = true;
}

void Entity::MulQuaternion(const float& x, const float& y, const float& z, const float& w)
{
	mQuaternion.Mul(x, y, z, w);
	mDirty = true;
}

const XMFLOAT4& Entity::GetQuaternion()
{
	return mQuaternion.Get();
}