#include "Position.h"

void Position::Set(const XMFLOAT3& position)
{
	mPosition = position;
}

void Position::Set(const float& x,const float& y,const float& z)
{
	mPosition = { x,y,z };
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
* a�� ������ ��� ��ҿ� �����ؼ� ���͸� �����ϰ� �̸� b�� ���� �Ŀ� position�� ���Ѵ�.
*/
void Position::MulAdd(const float& a, const XMFLOAT3& b)
{
	XMVECTOR d = XMVectorReplicate(a);
	XMVECTOR l = XMLoadFloat3(&b);
	XMVECTOR p = XMLoadFloat3(&mPosition);

	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(d, l, p));
}