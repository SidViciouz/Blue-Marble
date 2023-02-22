#include "Position.h"

void Position::Set(const XMFLOAT3& position)
{
	mPosition = position;
}

void Position::Set(const float& x,const float& y,const float& z)
{
	mPosition = { x,y,z };
}

void Position::Add(const XMFLOAT3& d)
{
	mPosition.x += d.x;
	mPosition.y += d.y;
	mPosition.z += d.z;
}

void Position::Add(const float& x,const float& y,const float& z)
{
	mPosition.x += x;
	mPosition.y += y;
	mPosition.z += z;
}