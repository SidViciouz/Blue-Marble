#pragma once

#include "../Util.h"

class Position
{
private:
	XMFLOAT3									mPosition;

public:
												Position();
	Position									operator+(const Position& other) const;

	void										Set(const XMFLOAT3& position);
	void										Set(const float& x,const float& y,const float& z);

	const XMFLOAT3&								Get();

	void										Add(const XMFLOAT3& d);
	void										Add(const float& x,const float& y,const float& z);

	void										MulAdd(const float& a,const XMFLOAT3& b);
};