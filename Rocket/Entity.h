#pragma once

#include "Util.h"
#include "Math/Quaternion.h"
#include "Math/Position.h"

class Entity
{
private:
	Position mPosition;
	Quaternion mQuaternion;

public:
	void SetPosition(const XMFLOAT3& position);
	void SetPosition(const float& x, const float& y, const float& z);
	const XMFLOAT3& GetPosition();

	void AddPosition(const XMFLOAT3& d);
	void AddPosition(const float& x,const float& y,const float& z);

	void SetQuaternion(const XMFLOAT4& quaternion);
	void SetQuaternion(const float& x, const float& y, const float& z, const float& w);
	const XMFLOAT4& GetQuaternion();
};
