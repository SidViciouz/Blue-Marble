#pragma once

#include "../Util.h"

class Position
{
private:
	XMFLOAT3 mPosition;

public:
	void Set(const XMFLOAT3& position);
	void Set(const float& x,const float& y,const float& z);

	const XMFLOAT3& Get();

	void Add(const XMFLOAT3& d);
	void Add(const float& x,const float& y,const float& z);
};