#pragma once
#include "../Util.h"

class Quaternion
{
private:
	XMFLOAT4 mQuaternion;

public:
	void Set(const XMFLOAT4& quaternion);
	void Set(const float& x,const float& y,const float& z,const float& w);
};
