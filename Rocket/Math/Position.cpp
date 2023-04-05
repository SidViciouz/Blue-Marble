#include "Position.h"

Position::Position()
{
	mPosition.x = 0.0f;
	mPosition.y = 0.0f;
	mPosition.z = 0.0f;
}

Position Position::operator+(const Position& other) const
{
	Position position;
	position.Set(mPosition.x + other.mPosition.x, mPosition.y + other.mPosition.y, mPosition.z + other.mPosition.z);

	return position;
}

Position Position::operator-(const Position& other) const
{
	Position position;
	position.Set(mPosition.x - other.mPosition.x, mPosition.y - other.mPosition.y, mPosition.z - other.mPosition.z);

	return position;
}

void Position::Set(const XMFLOAT3& position)
{
	mPosition = position;
}

void Position::Set(const float& x,const float& y,const float& z)
{
	mPosition.x = x;
	mPosition.y = y;
	mPosition.z = z;
}

const XMFLOAT3& Position::Get()
{
	return mPosition;
}

void Position::Add(const XMFLOAT3& d)
{
	XMVECTOR a = XMLoadFloat3(&mPosition);
	XMVECTOR b = XMLoadFloat3(&d);

	XMStoreFloat3(&mPosition, a + b);
}

void Position::Add(const float& x,const float& y,const float& z)
{
	XMVECTOR a = XMLoadFloat3(&mPosition);
	XMVECTOR b = XMVectorSet(x, y, z, 1.0f);

	XMStoreFloat3(&mPosition, a + b);
}

/*
* a를 벡터의 모든 요소에 복제해서 벡터를 생성하고 이를 b에 곱한 후에 position과 더한다.
*/
void Position::MulAdd(const float& a, const XMFLOAT3& b)
{
	XMVECTOR d = XMVectorReplicate(a);
	XMVECTOR l = XMLoadFloat3(&b);
	XMVECTOR p = XMLoadFloat3(&mPosition);

	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(d, l, p));
}